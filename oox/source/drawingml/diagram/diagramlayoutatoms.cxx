/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagramlayoutatoms.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:58 $
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

#include "oox/drawingml/diagram/diagramlayoutatoms.hxx"

#include <functional>
#include <boost/bind.hpp>

#include "oox/helper/attributelist.hxx"
#include "layoutnodecontext.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace oox { namespace drawingml {


IteratorAttr::IteratorAttr( )
    : mnAxis( 0 )
    , mnCnt( 0 )
    , mbHideLastTrans( false )
    , mnPtType( 0 )
    , mnSt( 0 )
    , mnStep( 1 )
{
}

void IteratorAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    AttributeList attr( xAttr );
    mnAxis = xAttr->getOptionalValueToken( XML_axis, 0 );
    mnCnt = attr.getInteger( XML_cnt, 0 );
    mbHideLastTrans = attr.getBool( XML_hideLastTrans, false );
    mnPtType = xAttr->getOptionalValueToken( XML_ptType, 0 );
    mnSt = attr.getInteger( XML_st, 0 );
    mnStep = attr.getInteger( XML_step, 1 );
}



ConditionAttr::ConditionAttr()
    : mnFunc( 0 )
    , mnArg( 0 )
    , mnOp( 0 )
{

}


void ConditionAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    mnFunc = xAttr->getOptionalValueToken( XML_func, 0 );
    // mnArg will be -1 for "none" or any other unknown value
    mnArg = LayoutNodeContext::tagToVarIdx( xAttr->getOptionalValueToken( XML_arg, XML_none ) );
    mnOp = xAttr->getOptionalValueToken( XML_op, 0 );
    msVal = xAttr->getOptionalValue( XML_val );
}


void LayoutAtom::dump(int level)
{
    OSL_TRACE( "level = %d - %s of type %s", level,
               OUSTRING_TO_CSTR( msName ),
               typeid(*this).name() );
    std::for_each( mpChildNodes.begin(), mpChildNodes.end(),
                  boost::bind( &LayoutAtom::dump, _1, level + 1 ) );
}


void ForEachAtom::processAtom()
{
    // TODO there is likely some conditions
    std::for_each( mpChildNodes.begin(), mpChildNodes.end(),
                   boost::bind( &LayoutAtom::processAtom, _1 ) );
}

/** call ConditionAtom::test() if pAtom is one
 * if it is not a ConditionAtom, then return false.
 */
static bool _test_atom( const LayoutAtomPtr & pAtom)
{
    try {
        bool bResult = false;
        const ConditionAtomPtr pCond = boost::dynamic_pointer_cast< ConditionAtom >(pAtom);
        if( pCond )
        {
            bResult = pCond->test();
        }
        return bResult;
    }
    catch(...)
    {
    }
    return false;
}

void ChooseAtom::processAtom()
{
    std::vector< LayoutAtomPtr >::iterator
        iter = std::find_if( mpChildNodes.begin(), mpChildNodes.end(),
                             boost::bind( &_test_atom, _1 ) );
    if( iter != mpChildNodes.end() )
    {
        // TODO do something
        (*iter)->processAtom();
    }
}

bool ConditionAtom::test()
{
    // TODO
    return false;
}


} }
