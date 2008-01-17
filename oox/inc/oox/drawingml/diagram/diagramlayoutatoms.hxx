/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagramlayoutatoms.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/





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
