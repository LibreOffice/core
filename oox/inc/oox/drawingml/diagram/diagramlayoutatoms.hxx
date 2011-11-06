/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/







#ifndef OOX_DRAWINGML_DIAGRAMLAYOUTATOMS_HXX
#define OOX_DRAWINGML_DIAGRAMLAYOUTATOMS_HXX

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>

#include "oox/drawingml/shape.hxx"


namespace oox { namespace drawingml {


// AG_IteratorAttributes
class IteratorAttr
{
public:
    IteratorAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

private:
    sal_Int32 mnAxis;
    sal_Int32 mnCnt;
    sal_Bool  mbHideLastTrans;
    sal_Int32 mnPtType;
    sal_Int32 mnSt;
    sal_Int32 mnStep;
};

class ConditionAttr
{
public:
    ConditionAttr();

    // not sure this belong here, but wth
    void loadFromXAttr( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& xAttributes );

private:
    sal_Int32 mnFunc;
    sal_Int32 mnArg;
    sal_Int32 mnOp;
    ::rtl::OUString msVal;
};

class LayoutAtom;

typedef boost::shared_ptr< LayoutAtom > LayoutAtomPtr;

/** abstract Atom for the layout */
class LayoutAtom
{
public:
    virtual ~LayoutAtom()
        {}
    // TODO change signature to the proper one
    virtual void processAtom() = 0;
    void setName( const ::rtl::OUString & sName )
        { msName = sName; }
    void addChild( const LayoutAtomPtr & pNode )
        { mpChildNodes.push_back( pNode ); }

    // dump for debug
    virtual void dump(int level = 0);
protected:
    std::vector< LayoutAtomPtr > mpChildNodes;
    ::rtl::OUString msName;
};

class AlgAtom
    : public LayoutAtom
{
public:
    virtual ~AlgAtom()
        {}
    typedef std::map< std::string, ::com::sun::star::uno::Any > ParamMap;

    virtual void processAtom()
        {}
private:
    ParamMap mParams;
};


class ForEachAtom
    : public LayoutAtom
{
public:
    virtual ~ForEachAtom()
        {}

    IteratorAttr & iterator()
        { return maIter; }
    virtual void processAtom();
private:
    IteratorAttr maIter;
};

typedef boost::shared_ptr< ForEachAtom > ForEachAtomPtr;


class ConditionAtom
    : public LayoutAtom
{
public:
    ConditionAtom( bool bElse = false )
        : LayoutAtom( )
        , mbElse( bElse )
        {}
    virtual ~ConditionAtom()
        {}
    bool test();
    virtual void processAtom()
        {}
    IteratorAttr & iterator()
        { return maIter; }
    ConditionAttr & cond()
        { return maCond; }
private:
    bool          mbElse;
    IteratorAttr  maIter;
    ConditionAttr maCond;
};

typedef boost::shared_ptr< ConditionAtom > ConditionAtomPtr;


/** "choose" statements. Atoms will be tested in order. */
class ChooseAtom
    : public LayoutAtom
{
public:
    virtual ~ChooseAtom()
        {}
    virtual void processAtom();
};

class LayoutNode
    : public LayoutAtom
{
public:
    enum {
        VAR_animLvl = 0,
        VAR_animOne,
        VAR_bulletEnabled,
        VAR_chMax,
        VAR_chPref,
        VAR_dir,
        VAR_hierBranch,
        VAR_orgChart,
        VAR_resizeHandles
    };
    // we know that the array is of fixed size
    // the use of Any allow having empty values
    typedef boost::array< ::com::sun::star::uno::Any, 9 > VarMap;

    virtual ~LayoutNode()
        {}
    virtual void processAtom()
        {}
    VarMap & variables()
        { return mVariables; }
private:
    VarMap                       mVariables;
    std::vector< ShapePtr >      mpShapes;
};

typedef boost::shared_ptr< LayoutNode > LayoutNodePtr;

} }

#endif
