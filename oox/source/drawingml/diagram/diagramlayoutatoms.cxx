/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
