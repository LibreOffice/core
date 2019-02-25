/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <map>
#include <memory>

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include <oox/drawingml/shape.hxx>
#include "diagram.hxx"

namespace oox { namespace drawingml {

class DiagramLayout;
typedef std::shared_ptr< DiagramLayout > DiagramLayoutPtr;

// AG_IteratorAttributes
struct IteratorAttr
{
    IteratorAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes );

    sal_Int32 mnAxis;
    sal_Int32 mnCnt;
    bool  mbHideLastTrans;
    sal_Int32 mnPtType;
    sal_Int32 mnSt;
    sal_Int32 mnStep;
};

struct ConditionAttr
{
    ConditionAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes );

    sal_Int32 mnFunc;
    sal_Int32 mnArg;
    sal_Int32 mnOp;
    OUString msVal;
    sal_Int32 mnVal;
};

struct Constraint
{
    sal_Int32 mnFor;
    OUString msForName;
    sal_Int32 mnPointType;
    sal_Int32 mnType;
    sal_Int32 mnRefFor;
    OUString msRefForName;
    sal_Int32 mnRefType;
    sal_Int32 mnRefPointType;
    double mfFactor;
    double mfValue;
    sal_Int32 mnOperator;
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
    virtual const std::vector<LayoutAtomPtr>& getChildren() const
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
    void parseConstraint(std::vector<Constraint>& rConstraints, bool bRequireForName) const;
private:
    Constraint maConstraint;
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
    void addParam( sal_Int32 nType, sal_Int32 nVal )
        { maMap[nType]=nVal; }
    void layoutShape( const ShapePtr& rShape,
                      const std::vector<Constraint>& rConstraints );

    /// Gives access to <dgm:alg type="..."/>.
    sal_Int32 getType() const { return mnType; }

    /// Gives access to <dgm:param type="..." val="..."/>.
    const ParamMap& getMap() const { return maMap; }

    void setAspectRatio(double fAspectRatio) { mfAspectRatio = fAspectRatio; }

    double getAspectRatio() const { return mfAspectRatio; }

private:
    sal_Int32 mnType;
    ParamMap  maMap;
    /// Aspect ratio is not integer, so not part of maMap.
    double mfAspectRatio = 0;
};

typedef std::shared_ptr< AlgAtom > AlgAtomPtr;

class ForEachAtom
    : public LayoutAtom
{
public:
    explicit ForEachAtom(LayoutNode& rLayoutNode, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes);

    IteratorAttr & iterator()
        { return maIter; }
    virtual void accept( LayoutAtomVisitor& ) override;

private:
    IteratorAttr maIter;
};

typedef std::shared_ptr< ForEachAtom > ForEachAtomPtr;

class ConditionAtom
    : public LayoutAtom
{
public:
    explicit ConditionAtom(LayoutNode& rLayoutNode, bool isElse, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttributes);
    virtual void accept( LayoutAtomVisitor& ) override;
    bool getDecision() const;
private:
    static bool compareResult(sal_Int32 nOperator, sal_Int32 nFirst, sal_Int32 nSecond);
    const dgm::Point* getPresNode() const;
    sal_Int32 getNodeCount() const;

    bool const    mIsElse;
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
#if defined __clang__ && __clang_major__ == 3 && __clang_minor__ == 8
        , maEmptyChildren()
#endif
    {}
    virtual void accept( LayoutAtomVisitor& ) override;
    virtual const std::vector<LayoutAtomPtr>& getChildren() const override;
private:
    const std::vector<LayoutAtomPtr> maEmptyChildren;
};

class LayoutNode
    : public LayoutAtom
{
public:
    typedef std::map<sal_Int32, OUString> VarMap;

    LayoutNode(const Diagram& rDgm) : LayoutAtom(*this), mrDgm(rDgm), mnChildOrder(0) {}
    const Diagram& getDiagram() const
        { return mrDgm; }
    virtual void accept( LayoutAtomVisitor& ) override;
    VarMap & variables()
        { return mVariables; }
    void setMoveWith( const OUString & sName )
        { msMoveWith = sName; }
    void setStyleLabel( const OUString & sLabel )
        { msStyleLabel = sLabel; }
    void setChildOrder( sal_Int32 nOrder )
        { mnChildOrder = nOrder; }
    void setExistingShape( const ShapePtr& pShape )
        { mpExistingShape = pShape; }
    const ShapePtr& getExistingShape() const
        { return mpExistingShape; }
    const std::vector<ShapePtr> & getNodeShapes() const
        { return mpNodeShapes; }
    void addNodeShape(const ShapePtr& pShape)
        { mpNodeShapes.push_back(pShape); }

    bool setupShape( const ShapePtr& rShape,
                     const dgm::Point* pPresNode ) const;

    const LayoutNode* getParentLayoutNode() const;

    void setAlgAtom(AlgAtomPtr pAlgAtom) { mpAlgAtom = pAlgAtom; }

    AlgAtomPtr getAlgAtom() const { return mpAlgAtom.lock(); }

private:
    const Diagram&               mrDgm;
    VarMap                       mVariables;
    OUString                     msMoveWith;
    OUString                     msStyleLabel;
    ShapePtr                     mpExistingShape;
    std::vector<ShapePtr>        mpNodeShapes;
    sal_Int32                    mnChildOrder;
    std::weak_ptr<AlgAtom>       mpAlgAtom;
};

typedef std::shared_ptr< LayoutNode > LayoutNodePtr;

class ShapeAtom
    : public LayoutAtom
{
public:
    ShapeAtom(LayoutNode& rLayoutNode, const ShapePtr& pShape) : LayoutAtom(rLayoutNode), mpShapeTemplate(pShape) {}
    virtual void accept( LayoutAtomVisitor& ) override;
    const ShapePtr& getShapeTemplate() const
        { return mpShapeTemplate; }

private:
    ShapePtr const mpShapeTemplate;
};

typedef std::shared_ptr< ShapeAtom > ShapeAtomPtr;

struct LayoutAtomVisitor
{
    virtual ~LayoutAtomVisitor() {}
    virtual void visit(ConstraintAtom& rAtom) = 0;
    virtual void visit(AlgAtom& rAtom) = 0;
    virtual void visit(ForEachAtom& rAtom) = 0;
    virtual void visit(ConditionAtom& rAtom) = 0;
    virtual void visit(ChooseAtom& rAtom) = 0;
    virtual void visit(LayoutNode& rAtom) = 0;
    virtual void visit(ShapeAtom& rAtom) = 0;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
