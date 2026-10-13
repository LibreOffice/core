/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAMLAYOUTATOMS_HXX
#define INCLUDED_OOX_SOURCE_DRAWINGML_DIAGRAM_DIAGRAMLAYOUTATOMS_HXX

#include <limits>
#include <map>
#include <memory>
#include <optional>

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <utility>

#include "diagram.hxx"

namespace oox::drawingml {

typedef std::shared_ptr< DiagramLayout > DiagramLayoutPtr;

// AG_IteratorAttributes
struct IteratorAttr
{
    IteratorAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes );

    std::vector<sal_Int32> maAxis;

    // One entry per step of the axis. A start is the place of the first Point the step keeps,
    // counted from one. A count is how many it keeps from there, zero for all of them.
    std::vector<sal_Int32> maStart;
    std::vector<sal_Int32> maCount;

    // The kind of Point each step of the axis keeps, XML_all for every kind.
    std::vector<sal_Int32> maPtType;

    sal_Int32 mnCnt;
    bool  mbHideLastTrans;
    sal_Int32 mnPtType;
    sal_Int32 mnSt;
    sal_Int32 mnStep;
};

/// Gives the nodes a layout node states as alike, "h for des ptType=node op=equ" with no
/// reference, the smallest size among them along that axis. Runs once all shapes have their
/// size.
void settleEqualNodeSizes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot);

/// Gives every child whose side is bound both ways to a part of a named shape levels below a
/// sibling that part of the shape's side. Runs once all shapes have their size.
void settleSidesBoundToNamedShapes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot);

/// Puts every connector that names the shape it starts at and the one it ends at between those
/// two across the flow. Runs once all shapes have their place.
void settleNamedConnectors(const SmartArtDiagram& rDgm);

/// Gives every spacer whose extent is a part of a shape that a hierarchy beside it laid out that
/// extent now, and moves what follows it in its row along by it. Runs once the hierarchies
/// have their shapes.
void settleDeferredSpacers(const SmartArtDiagram& rDgm);

/// Turns the text of every turned shape back upright, or the way autoTxRot of the text
/// algorithm that styles the shape says. Runs once every shape has its turn.
void settleUprightText(const SmartArtDiagram& rDgm);

/// Moves the drawn shapes as a whole so that they stand in the middle of the frame, along each
/// axis the root's algorithm states no alignment for. Runs once all shapes have their place.
void centreDrawnShapes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot);

/// The Points the axis of rIterator picks out, starting at the Point with rFromId, in the order
/// the file puts them in. The start and the count each step of the axis carries are applied,
/// except on the last step when bLastStepWhole says to leave that one whole.
std::vector<OUString> pointsAlongAxis(const SmartArtDiagram& rDgm, const IteratorAttr& rIterator,
                                      const OUString& rFromId, bool bLastStepWhole);

struct ConditionAttr
{
    ConditionAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes );

    OUString msVal;
    sal_Int32 mnFunc;
    sal_Int32 mnArg;
    sal_Int32 mnOp;
    sal_Int32 mnVal;
};

/// Constraints allow you to specify an ideal (or starting point) size for each shape.
struct Constraint
{
    OUString msForName;
    OUString msRefForName;
    double mfFactor;
    double mfValue;
    sal_Int32 mnFor;
    sal_Int32 mnPointType;
    sal_Int32 mnType;
    sal_Int32 mnRefFor;
    sal_Int32 mnRefType;
    sal_Int32 mnRefPointType;
    sal_Int32 mnOperator;
    /// The name of the layout node the constraint is stated in. A reference that names no
    /// shape, refType w with no refFor, is that node's extent.
    OUString msStatedBy;
};

/// Rules allow you to specify what to do when constraints can't be fully satisfied.
struct Rule
{
    OUString msForName;
    /// what the rule may change, XML_w and the like, XML_none where it states nothing
    sal_Int32 mnType = 0;
    /// how far the rule may take the value: to a factor of what the constraint refers to, to a
    /// value of its own, and never beyond a limit; NaN for each where the rule states none
    double mfFactor = std::numeric_limits<double>::quiet_NaN();
    double mfValue = std::numeric_limits<double>::quiet_NaN();
    double mfMax = std::numeric_limits<double>::quiet_NaN();
};

typedef std::map<sal_Int32, sal_Int32> LayoutProperty;
typedef std::map<OUString, LayoutProperty> LayoutPropertyMap;

struct LayoutAtomVisitor;
class LayoutAtom;
class LayoutNode;

typedef std::shared_ptr< LayoutAtom > LayoutAtomPtr;

/** abstract Atom for the layout */
class LayoutAtom
{
public:
    LayoutAtom(LayoutNode& rLayoutNode) : mrLayoutNode(rLayoutNode) {}
    virtual ~LayoutAtom() { }

    LayoutNode& getLayoutNode()
        { return mrLayoutNode; }
    const LayoutNode& getLayoutNode() const
        { return mrLayoutNode; }

    /** visitor acceptance
     */
    virtual void accept( LayoutAtomVisitor& ) = 0;

    void setName( const OUString& sName )
        { msName = sName; }
    const OUString& getName() const
        { return msName; }

private:
    void addChild( const LayoutAtomPtr & pNode )
        { mpChildNodes.push_back( pNode ); }
    void setParent(const LayoutAtomPtr& pParent) { mpParent = pParent; }

public:
    const std::vector<LayoutAtomPtr>& getChildren() const
        { return mpChildNodes; }

    LayoutAtomPtr getParent() const { return mpParent.lock(); }

    static void connect(const LayoutAtomPtr& pParent, const LayoutAtomPtr& pChild)
    {
        pParent->addChild(pChild);
        pChild->setParent(pParent);
    }

    // dump for debug
    void dump(int level = 0);

protected:
    LayoutNode&            mrLayoutNode;
    std::vector< LayoutAtomPtr > mpChildNodes;
    std::weak_ptr<LayoutAtom> mpParent;
    OUString                     msName;
};

class ConstraintAtom
    : public LayoutAtom
{
public:
    ConstraintAtom(LayoutNode& rLayoutNode) : LayoutAtom(rLayoutNode) {}
    virtual void accept( LayoutAtomVisitor& ) override;
    Constraint& getConstraint()
        { return maConstraint; }
    const Constraint& getConstraint() const
        { return maConstraint; }
    void parseConstraint(std::vector<Constraint>& rConstraints, bool bRequireForName) const;
private:
    Constraint maConstraint;
};

/// Represents one <dgm:rule> element.
class RuleAtom
    : public LayoutAtom
{
public:
    RuleAtom(LayoutNode& rLayoutNode) : LayoutAtom(rLayoutNode) {}
    virtual void accept( LayoutAtomVisitor& ) override;
    Rule& getRule()
        { return maRule; }
    const Rule& getRule() const
        { return maRule; }
    void parseRule(std::vector<Rule>& rRules) const;
private:
    Rule maRule;
};

class AlgAtom
    : public LayoutAtom
{
public:
    AlgAtom(LayoutNode& rLayoutNode) : LayoutAtom(rLayoutNode), mnType(0), maMap() {}

    typedef std::map<sal_Int32,sal_Int32> ParamMap;

    virtual void accept( LayoutAtomVisitor& ) override;

    void setType( sal_Int32 nToken )
        { mnType = nToken; }
    sal_Int32 getType() const
        { return mnType; }
    const ParamMap& getMap() const { return maMap; }
    void addParam( sal_Int32 nType, sal_Int32 nVal )
        { maMap[nType]=nVal; }

    // A parameter whose value is the name of a layout node, srcNode and dstNode of a connector.
    void addNamedParam(sal_Int32 nType, const OUString& rName) { maNamedParams[nType] = rName; }
    OUString getNamedParam(sal_Int32 nType) const
    {
        const auto aFound = maNamedParams.find(nType);
        return aFound == maNamedParams.end() ? OUString() : aFound->second;
    }
    sal_Int32 getVerticalShapesCount(const ShapePtr& rShape);

    /// A hierarchy root beside its branch, sized from its constraints and fitted as a whole.
    /// True when it laid the root out.
    bool layoutSidewaysRoot(const ShapePtr& rShape, const std::vector<Constraint>& rConstraints);

    /// A branch of roots that stand beside their branches, the whole hierarchy below it sized
    /// from its constraints and fitted into the branch at one scale. True when it laid it out.
    bool layoutSidewaysBranch(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                              const std::vector<Constraint>& rConstraints);

    /// A row of roots that stand above their branches, the whole hierarchy below it sized from
    /// its constraints, packed level by level and fitted into the row at one scale. True when it
    /// laid it out.
    bool layoutUprightBranch(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                             const std::vector<Constraint>& rConstraints);
    void layoutShape( const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                      const std::vector<Constraint>& rConstraints,
                      const std::vector<Rule>& rRules );

    void setAspectRatio(double fAspectRatio) { mfAspectRatio = fAspectRatio; }

    double getAspectRatio() const { return mfAspectRatio; }

private:
    sal_Int32 mnType;
    ParamMap  maMap;
    std::map<sal_Int32, OUString> maNamedParams;
    /// Aspect ratio is not integer, so not part of maMap.
    double mfAspectRatio = 0;

    /// Determines the connector shape type for conn algorithm
    sal_Int32 getConnectorType();
};

typedef std::shared_ptr< AlgAtom > AlgAtomPtr;

/// Finds optimal grid to layout children that have fixed aspect ratio.
class SnakeAlg
{
public:
    static void layoutShapeChildren(const SmartArtDiagram& rDgm, const AlgAtom& rAlg,
                                    const ShapePtr& rShape,
                                    const std::vector<Constraint>& rConstraints);

    /// A snake whose transitions are drawn as connectors, a bending process. True when it
    /// laid the children out, false when this is not such a snake.
    static bool layoutBendingProcess(const AlgAtom& rAlg, const ShapePtr& rShape,
                                     const std::vector<Constraint>& rConstraints);
};

/**
 * Lays out child layout nodes along a vertical path and works with the trapezoid shape to create a
 * pyramid.
 */
class PyraAlg
{
public:
    static void layoutShapeChildren(const SmartArtDiagram& rDgm, AlgAtom& rAlg,
                                    const ShapePtr& rShape,
                                    const std::vector<Constraint>& rConstraints);
};

/**
 * Specifies the size and position for all child layout nodes.
 */
class CompositeAlg
{
public:
    static void layoutShapeChildren(const SmartArtDiagram& rDgm, AlgAtom& rAlg, const ShapePtr& rShape,
                                    const std::vector<Constraint>& rConstraints);

private:
    /**
     * Apply rConstraint to the rProperties shared layout state.
     *
     * Note that the order in which constraints are applied matters, given that constraints can refer to
     * each other, and in case A depends on B and A is applied before B, the effect of A won't be
     * updated when B is applied.
     */
    static void applyConstraintToLayout(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                                        const std::vector<Constraint>& rAll,
                                        const Constraint& rConstraint,
                                        LayoutPropertyMap& rProperties,
                                        std::map<sal_Int32, sal_Int32>& rUserVariables);

    /**
     * Decides if a certain reference type (e.g. "right") can be inferred from the available properties
     * in rMap (e.g. left and width). Returns true if rValue is written to.
     */
    static bool inferFromLayoutProperty(const LayoutProperty& rMap, sal_Int32 nRefType,
                                        sal_Int32& rValue);
};

class ForEachAtom
    : public LayoutAtom
{
public:
    explicit ForEachAtom(LayoutNode& rLayoutNode, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes);

    IteratorAttr & iterator()
        { return maIter; }
    void setRef(const OUString& rsRef)
        { msRef = rsRef; }
    const IteratorAttr& iterator() const
        { return maIter; }
    const OUString& getRef() const
        { return msRef; }
    virtual void accept( LayoutAtomVisitor& ) override;
    LayoutAtomPtr getRefAtom(const SmartArtDiagram& rDgm);

private:
    IteratorAttr maIter;
    OUString msRef;
};

typedef std::shared_ptr< ForEachAtom > ForEachAtomPtr;

class ConditionAtom
    : public LayoutAtom
{
public:
    explicit ConditionAtom(LayoutNode& rLayoutNode, bool isElse, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes);
    virtual void accept( LayoutAtomVisitor& ) override;
    /// rPassNodeId is the modelId of the Point the pass of the loop around this condition
    /// stands on, empty where there is no loop or it knows of none.
    bool getDecision(const SmartArtDiagram& rDgm,
                     const rtl::Reference<svx::diagram::Point>& rPresPoint,
                     const OUString& rPassNodeId) const;

private:
    static bool compareResult(sal_Int32 nOperator, sal_Int32 nFirst, sal_Int32 nSecond);
    sal_Int32 getNodeCount(const SmartArtDiagram& rDgm, const OUString& rNodeId) const;
    /// The place the data Point behind rPresPoint takes among its siblings, counted from one,
    /// and how many siblings there are. Both stay zero for a Point with no parent.
    static void getNodePlace(const SmartArtDiagram& rDgm, const OUString& rNodeId,
                             sal_Int32& rPosition, sal_Int32& rSiblings);

    bool          mIsElse;
    IteratorAttr  maIter;
    ConditionAttr maCond;
};

typedef std::shared_ptr< ConditionAtom > ConditionAtomPtr;

/** "choose" statements. Atoms will be tested in order. */
class ChooseAtom
    : public LayoutAtom
{
public:
    ChooseAtom(LayoutNode& rLayoutNode)
        : LayoutAtom(rLayoutNode)
    {}
    virtual void accept( LayoutAtomVisitor& ) override;
};

// I have cleaned LayoutNode from the member 'SmartArtDiagram& mrDgm'
// since LayoutNode is model data and with that in place it would have
// been necessary to deep-clone or re-import the complete DiagramLayout,
// just to make sure that those members of SmartArtDiagram& will
// reference the correct - newly created - SmartArtDiagram.
// That again would have been hard to keep under control due to
// LayoutNodes hosting a full hierarchy of LayoutNodes in it's base
// class LayoutAtom, so complicated deep-copy and need of virtual Clone
// method at LayoutAtom to do the right thing for all eight derivations
// of it.
// This is now no longer needed. It can just stay shared, thus the
// DiagramLayoutPtr gets just copied above. This reflects that the
// layout mechanism hosted by these atoms is not changed itself, but
// used (in some ForEach manner) to create the shapes, so does not
// need to be changed itself.
// It gets now just additionally referenced by the SmartArtDiagram copy
// costructor. That is for non-deep copy/paste where this copy operator
// is used.
// For deep copy the SmartArtDiagram constructor with import from
// boost::property_tree has to be used which will have to re-import
// the layout model data to mpLayout anyways.
// Instead of having SmartArtDiagram as member it now will be handed
// as needed to import contexts (LayoutNodeContext and derivatives).
// All those classes are used temporarily and are not part of the model.
// It also gets handed over for shape re-creation in LayoutAtomVisitorBase
// and it's derivates. Also those classes are used temporarily and are
// not part of the model.
class LayoutNode
    : public LayoutAtom
{
public:
    typedef std::map<sal_Int32, OUString> VarMap;

    LayoutNode();
    virtual void accept( LayoutAtomVisitor& ) override;
    VarMap & variables()
        { return mVariables; }
    void setMoveWith( const OUString & sName )
        { msMoveWith = sName; }
    void setStyleLabel( const OUString & sLabel )
        { msStyleLabel = sLabel; }
    const OUString & getStyleLabel() const
        { return msStyleLabel; }

    // What the shape of this layout node represents: which data Points it stands for and along which
    // axis they are reached. A layout node that holds no value of its own represents no data Point and
    // only gives the layout somewhere to put other nodes.
    void setPresentationOf( const IteratorAttr & rIterator )
        { moPresentationOf = rIterator; }
    const std::optional<IteratorAttr> & getPresentationOf() const
        { return moPresentationOf; }
    void setChildOrder( sal_Int32 nOrder )
        { mnChildOrder = nOrder; }
    sal_Int32 getChildOrder() const { return mnChildOrder; }
    void setExistingShape( const ShapePtr& pShape )
        { mpExistingShape = pShape; }
    const ShapePtr& getExistingShape() const
        { return mpExistingShape; }
    const std::vector<ShapePtr> & getNodeShapes() const
        { return mpNodeShapes; }
    void addNodeShape(const ShapePtr& pShape)
        { mpNodeShapes.push_back(pShape); }

    bool setupShape( const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                     const rtl::Reference<svx::diagram::Point>& rPresNode,
                     sal_Int32 nCurrIdx ) const;

    const LayoutNode* getParentLayoutNode() const;

private:
    VarMap                       mVariables;
    std::optional<IteratorAttr>  moPresentationOf;
    OUString                     msMoveWith;
    OUString                     msStyleLabel;
    ShapePtr                     mpExistingShape;
    std::vector<ShapePtr>        mpNodeShapes;
    sal_Int32                    mnChildOrder;
};

typedef std::shared_ptr< LayoutNode > LayoutNodePtr;

class ShapeAtom
    : public LayoutAtom
{
public:
    ShapeAtom(LayoutNode& rLayoutNode, ShapePtr pShape) : LayoutAtom(rLayoutNode), mpShapeTemplate(std::move(pShape)) {}
    virtual void accept( LayoutAtomVisitor& ) override;
    const ShapePtr& getShapeTemplate() const
        { return mpShapeTemplate; }

private:
    ShapePtr mpShapeTemplate;
};

typedef std::shared_ptr< ShapeAtom > ShapeAtomPtr;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
