/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "CoinMPModel.hxx"
#include "SolverComponent.hxx"
#include "strings.hrc"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <comphelper/scopeguard.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>
#include <float.h>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace com::sun::star;

namespace
{
// Sequential Linear Programming (SLP) solver for smooth nonlinear models.
// CoinMP can only solve straight-line (linear) problems, so this solver reaches
// a nonlinear optimum by solving a sequence of linear ones, each a fresh
// approximation of the real model around the latest point.
//
// Starting from the current cell values, it repeats four steps until the point
// stops moving:
//
// 1. Approximate the objective and every constraint cell with an affine model
//    around the current point. Each variable is nudged a little and the
//    response measured, which gives a constant plus a slope per variable. The
//    approximation is only trustworthy close to where it was sampled.
//
// 2. Solve that linear approximation with CoinMP, but only over a trust region:
//    a box around the current point, sized as a fraction of each variable's
//    scale, inside which the model is believed accurate. The solve returns a
//    trial point.
//
// 3. Score the trial point with a merit function: its real objective value
//    plus a large penalty for any constraint violation. Combining both goals
//    into one number lets two points be compared. The trial is accepted only if
//    its merit beats the current point, otherwise it is rejected.
//
// 4. Resize the trust region from how well the linear model predicted the real
//    change. A good prediction grows it so the next step can reach further. A
//    poor prediction, or a rejected trial, shrinks it so the model is resampled
//    in a smaller region where it is more reliable.
//
// The loop ends when the step becomes negligible, the trust region collapses,
// the iteration limit is reached, or the time budget runs out. Success is
// reported only when the final point actually satisfies the constraints.
//
// The method is standard. Griffith and Stewart introduced sequential linear
// programming in "A nonlinear programming technique for the optimization of
// continuous processing systems", Management Science 7 (1961), 379-392. The
// trust region, the actual-to-predicted reduction ratio that resizes it, and
// the penalty merit function are covered by Nocedal and Wright, Numerical
// Optimization, second edition (Springer, 2006).
class SLPSolver : public SolverComponent
{
public:
    SLPSolver() {}

private:
    virtual void SAL_CALL solve() override;
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.Calc.SLPSolver"_ustr;
    }
    virtual OUString SAL_CALL getComponentDescription() override
    {
        return SolverComponent::GetResourceString(RID_SLP_SOLVER_COMPONENT);
    }

    void applyPoint(const std::vector<double>& rPoint);
    // Raw objective value at the current cell values.
    double objectiveAt();
    // Total amount by which the current cell values break the constraints.
    double violationAt();
    // Value to minimize: the objective for a minimize model, its negation for a
    // maximize model.
    double cost(double fObjective) const { return mbMaximize ? -fObjective : fObjective; }
    // Merit score of a point: its cost plus fPenalty for each unit of constraint
    // violation. Leaves the variable cells holding rPoint.
    double meritAt(const std::vector<double>& rPoint, double fPenalty);

    // Fill rLower and rUpper from the single-variable constant constraints and
    // the non-negative option.
    void deriveBounds(std::vector<double>& rLower, std::vector<double>& rUpper);
    // A representative magnitude for each variable, used to size the trust region
    // and the finite-difference step.
    std::vector<double> deriveScale(const std::vector<double>& rLower,
                                    const std::vector<double>& rUpper,
                                    const std::vector<double>& rStart);
    // Build an affine model of the objective and every constraint cell around
    // rPoint. For each dependent cell the returned vector holds the model's value
    // extrapolated to the origin (all variables zero) at index 0, and the slope
    // along variable j from a finite difference of size rStep[j] at index j+1.
    // Leaves the variable cells holding rPoint.
    ScSolverCellHashMap sampleAffineModel(const std::vector<double>& rPoint,
                                          const std::vector<double>& rStep);
    // Sample the model around rPoint and solve it over a trust region of the
    // given radius, returning the linear subproblem's result.
    CoinMpSolveResult
    solveLinearizedStep(const std::vector<double>& rPoint, const std::vector<double>& rScale,
                        const std::vector<double>& rLower, const std::vector<double>& rUpper,
                        const std::vector<char>& rColumnType, double fTrust, double fTimeout);
    // Store rPoint as the solution when it satisfies the constraints, otherwise
    // record that the model is infeasible.
    void reportSolution(const std::vector<double>& rPoint);
};

void SLPSolver::applyPoint(const std::vector<double>& rPoint)
{
    for (size_t i = 0; i < maVariables.size(); ++i)
        SetValue(maVariables[i], rPoint[i]);
}

double SLPSolver::objectiveAt() { return GetValue(maObjective); }

double SLPSolver::violationAt()
{
    double fTotal = 0.0;
    for (const auto& rConstraint : maConstraints)
    {
        sheet::SolverConstraintOperator eOperator = rConstraint.Operator;
        if (eOperator != sheet::SolverConstraintOperator_LESS_EQUAL
            && eOperator != sheet::SolverConstraintOperator_GREATER_EQUAL
            && eOperator != sheet::SolverConstraintOperator_EQUAL)
            continue;

        double fLeft = GetValue(rConstraint.Left);
        double fRight = 0.0;
        table::CellAddress aRightAddress;
        if (rConstraint.Right >>= aRightAddress)
            fRight = GetValue(aRightAddress);
        else if (!(rConstraint.Right >>= fRight))
            continue;

        double fExcess;
        if (eOperator == sheet::SolverConstraintOperator_LESS_EQUAL)
            fExcess = fLeft - fRight; // Feasible when not positive.
        else if (eOperator == sheet::SolverConstraintOperator_GREATER_EQUAL)
            fExcess = fRight - fLeft;
        else
            fExcess = std::abs(fLeft - fRight); // Equality: any difference is a violation.

        if (fExcess > 0.0)
            fTotal += fExcess;
    }
    return fTotal;
}

double SLPSolver::meritAt(const std::vector<double>& rPoint, double fPenalty)
{
    applyPoint(rPoint);
    return cost(objectiveAt()) + fPenalty * violationAt();
}

void SLPSolver::deriveBounds(std::vector<double>& rLower, std::vector<double>& rUpper)
{
    const size_t nVariables = maVariables.size();
    rLower.assign(nVariables, mbNonNegative ? 0.0 : -DBL_MAX);
    rUpper.assign(nVariables, DBL_MAX);

    // A constant on the right and the variable on the left is a plain bound. It
    // is still passed to the subproblem as an ordinary row as well.
    for (const auto& rConstraint : maConstraints)
    {
        sheet::SolverConstraintOperator eOperator = rConstraint.Operator;
        double fRhs = 0.0;
        if (!(rConstraint.Right >>= fRhs))
            continue;
        for (size_t j = 0; j < nVariables; ++j)
        {
            if (!AddressEqual(maVariables[j], rConstraint.Left))
                continue;
            if (eOperator == sheet::SolverConstraintOperator_LESS_EQUAL && fRhs < rUpper[j])
                rUpper[j] = fRhs;
            else if (eOperator == sheet::SolverConstraintOperator_GREATER_EQUAL && fRhs > rLower[j])
                rLower[j] = fRhs;
            else if (eOperator == sheet::SolverConstraintOperator_EQUAL)
            {
                rLower[j] = fRhs;
                rUpper[j] = fRhs;
            }
        }
    }
}

std::vector<double> SLPSolver::deriveScale(const std::vector<double>& rLower,
                                           const std::vector<double>& rUpper,
                                           const std::vector<double>& rStart)
{
    const size_t nVariables = maVariables.size();
    std::vector<double> aScale(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
    {
        // The width of a finite range, otherwise the size of the starting value.
        if (rLower[j] > -DBL_MAX && rUpper[j] < DBL_MAX && rUpper[j] > rLower[j])
            aScale[j] = rUpper[j] - rLower[j];
        else
            aScale[j] = std::max(1.0, std::abs(rStart[j]));
    }
    return aScale;
}

ScSolverCellHashMap SLPSolver::sampleAffineModel(const std::vector<double>& rPoint,
                                                 const std::vector<double>& rStep)
{
    const size_t nVariables = maVariables.size();

    // The dependent cells are the objective and each side of every constraint.
    ScSolverCellHashMap aCells;
    aCells[maObjective].reserve(nVariables + 1);
    for (const auto& rConstraint : maConstraints)
    {
        aCells[rConstraint.Left].reserve(nVariables + 1);
        table::CellAddress aRightAddress;
        if (rConstraint.Right >>= aRightAddress)
            aCells[aRightAddress].reserve(nVariables + 1);
    }

    // Value of every dependent cell at the base point, stored at index 0 for now.
    for (size_t j = 0; j < nVariables; ++j)
        SetValue(maVariables[j], rPoint[j]);
    for (auto& rEntry : aCells)
        rEntry.second.push_back(GetValue(rEntry.first));

    // Perturb one variable at a time to estimate the slope of every cell.
    for (size_t j = 0; j < nVariables; ++j)
    {
        SetValue(maVariables[j], rPoint[j] + rStep[j]);
        for (auto& rEntry : aCells)
        {
            double fBase = rEntry.second.front();
            double fPerturbed = GetValue(rEntry.first);
            rEntry.second.push_back((fPerturbed - fBase) / rStep[j]);
        }
        SetValue(maVariables[j], rPoint[j]);
    }

    // Turn the base value at index 0 into the model's value at the origin:
    // f(0) = f(point) - sum_j (slope_j * point_j).
    for (auto& rEntry : aCells)
    {
        double fAtOrigin = rEntry.second.front();
        for (size_t j = 0; j < nVariables; ++j)
            fAtOrigin -= rEntry.second[j + 1] * rPoint[j];
        rEntry.second.front() = fAtOrigin;
    }

    return aCells;
}

CoinMpSolveResult
SLPSolver::solveLinearizedStep(const std::vector<double>& rPoint, const std::vector<double>& rScale,
                               const std::vector<double>& rLower, const std::vector<double>& rUpper,
                               const std::vector<char>& rColumnType, double fTrust, double fTimeout)
{
    const size_t nVariables = maVariables.size();

    // Finite-difference step: a small fraction of each variable's scale, with an
    // absolute floor so a near-zero scale is still probed.
    constexpr double fRelativeStep = 1.0e-6;
    constexpr double fStepFloor = 1.0e-7;
    std::vector<double> aStep(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
        aStep[j] = std::max(fStepFloor, fRelativeStep * rScale[j]);

    ScSolverCellHashMap aCells = sampleAffineModel(rPoint, aStep);

    // Trust-region bounds, kept inside the real variable bounds.
    std::vector<double> aTrustLower(nVariables);
    std::vector<double> aTrustUpper(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
    {
        aTrustLower[j] = std::max(rLower[j], rPoint[j] - fTrust * rScale[j]);
        aTrustUpper[j] = std::min(rUpper[j], rPoint[j] + fTrust * rScale[j]);
        if (aTrustUpper[j] < aTrustLower[j])
            aTrustUpper[j] = aTrustLower[j];
    }

    return coinmpSolveLinearModel(maVariables, maConstraints, maObjective, aCells, aTrustLower,
                                  aTrustUpper, rColumnType, mbMaximize, fTimeout);
}

void SLPSolver::reportSolution(const std::vector<double>& rPoint)
{
    applyPoint(rPoint);
    double fObjective = objectiveAt();
    double fViolation = violationAt();

    // Report success only when the final point really satisfies the model.
    // The tolerance scales with the objective magnitude.
    constexpr double fFeasibilityTolerance = 1.0e-6;
    bool bFeasible = fViolation <= fFeasibilityTolerance * (1.0 + std::abs(fObjective));
    mbSuccess = bFeasible;
    if (bFeasible)
    {
        maSolution.realloc(maVariables.size());
        std::copy(rPoint.begin(), rPoint.end(), maSolution.getArray());
        mfResultValue = fObjective;
    }
    else
    {
        maStatus = SolverComponent::GetResourceString(RID_ERROR_INFEASIBLE);
    }
}

void SAL_CALL SLPSolver::solve()
{
    uno::Reference<frame::XModel> xModel(mxDoc, uno::UNO_QUERY_THROW);

    maStatus.clear();
    mbSuccess = false;

    const size_t nVariables = maVariables.size();
    if (nVariables == 0)
        return;

    xModel->lockControllers();
    comphelper::ScopeGuard aUnlockGuard([&xModel] { xModel->unlockControllers(); });

    std::vector<double> aLower;
    std::vector<double> aUpper;
    deriveBounds(aLower, aUpper);

    // Starting point: the current cell values, clamped to the bounds.
    std::vector<double> aX(nVariables);
    for (size_t j = 0; j < nVariables; ++j)
        aX[j] = std::clamp(GetValue(maVariables[j]), aLower[j], aUpper[j]);

    std::vector<double> aScale = deriveScale(aLower, aUpper, aX);
    std::vector<char> aColumnType(nVariables, mbInteger ? 'I' : 'C');

    // SLP tuning parameters. The trust region is a fraction of each variable's
    // scale: it grows by fTrustGrow when the model predicts a step well (ratio
    // above fGoodRatio) and shrinks by fTrustShrink otherwise (ratio below
    // fPoorRatio, or a rejected step). The loop stops once the relative step is
    // below fStepTolerance or the radius falls below fTrustMin.
    constexpr double fPenalty = 1.0e4;
    constexpr double fInitialTrust = 0.3;
    constexpr double fTrustMin = 1.0e-9;
    constexpr double fTrustMax = 1.0;
    constexpr double fTrustGrow = 2.0;
    constexpr double fTrustShrink = 0.5;
    constexpr double fGoodRatio = 0.75;
    constexpr double fPoorRatio = 0.25;
    constexpr double fStepTolerance = 1.0e-7;
    constexpr double fMinPredictedReduction = 1.0e-12;
    constexpr double fDefaultTimeout = 100.0;
    // High safety limit only. The time budget and the convergence test below
    // are what normally end the loop, so a large model is not cut off early.
    constexpr sal_Int32 nMaxIterations = 100000;

    double fTrust = fInitialTrust;
    double fMerit = meritAt(aX, fPenalty);

    const double fTimeout = mnTimeout > 0 ? double(mnTimeout) : fDefaultTimeout;
    auto aStart = std::chrono::steady_clock::now();

    for (sal_Int32 nIteration = 0; nIteration < nMaxIterations; ++nIteration)
    {
        std::chrono::duration<double> aElapsed = std::chrono::steady_clock::now() - aStart;
        if (aElapsed.count() >= fTimeout)
            break;

        CoinMpSolveResult aSubproblem
            = solveLinearizedStep(aX, aScale, aLower, aUpper, aColumnType, fTrust, fTimeout);
        if (!aSubproblem.bSuccess)
        {
            // The linearized problem could not be solved in this trust region.
            fTrust *= fTrustShrink;
            if (fTrust < fTrustMin)
                break;
            continue;
        }

        const std::vector<double>& rTrial = aSubproblem.aSolution;
        double fMeritTrial = meritAt(rTrial, fPenalty);

        // The subproblem meets the linearized constraints, so its predicted
        // violation is zero.
        double fPredictedMerit = cost(aSubproblem.fObjective);
        double fActualReduction = fMerit - fMeritTrial;
        double fPredictedReduction = fMerit - fPredictedMerit;

        double fStepRelative = 0.0;
        for (size_t j = 0; j < nVariables; ++j)
            fStepRelative = std::max(fStepRelative, std::abs(rTrial[j] - aX[j]) / aScale[j]);

        if (fActualReduction > 0.0)
        {
            // The trial improved the merit, so move to it.
            aX = rTrial;
            fMerit = fMeritTrial;

            // Grow the trust region when the model predicted the change well,
            // shrink it when the prediction was poor.
            double fRatio = fPredictedReduction > fMinPredictedReduction
                                ? fActualReduction / fPredictedReduction
                                : 1.0;
            if (fRatio > fGoodRatio)
                fTrust = std::min(fTrustMax, fTrust * fTrustGrow);
            else if (fRatio < fPoorRatio)
                fTrust *= fTrustShrink;
        }
        else
        {
            // The trial did not improve the merit, so reject it and shrink the
            // trust region.
            fTrust *= fTrustShrink;
        }

        if (fStepRelative < fStepTolerance || fTrust < fTrustMin)
            break;
    }

    reportSolution(aX);
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Calc_SLPSolver_get_implementation(css::uno::XComponentContext*,
                                                    css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SLPSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
