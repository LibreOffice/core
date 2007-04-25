/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapperTableManager.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-04-25 11:27:43 $
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
#include <DomainMapperTableManager.hxx>
#include <doctok/WW8ResourceModel.hxx>
#ifndef INCLUDED_BORDERHANDLER_HXX
#include <BorderHandler.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_HORIORIENTATION_HDL_
#include <com/sun/star/text/HoriOrientation.hpp>
#endif

namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::DomainMapperTableManager()
{
}
/*-- 23.04.2007 14:57:49---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapperTableManager::~DomainMapperTableManager()
{
}
/*-- 23.04.2007 15:25:37---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapperTableManager::sprm(doctok::Sprm & rSprm)
{
    bool bRet = DomainMapperTableManager_Base_t::sprm(rSprm);
    if( !bRet )
    {
        bRet = true;
        sal_uInt32 nId = rSprm.getId();
        doctok::Value::Pointer_t pValue = rSprm.getValue();
        sal_Int32 nIntValue = pValue->getInt();
        /* WRITERFILTERSTATUS: table: table_sprmdata */
        switch( nId )
        {
            case 0x5400: // sprmTJc
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                //table justification 0: left, 1: center, 2: right
                sal_Int16 nOrient = text::HoriOrientation::LEFT;
                switch( nIntValue )
                {
                    case 1 : nOrient = text::HoriOrientation::CENTER; break;
                    case 2 : nOrient = text::HoriOrientation::RIGHT; break;
                    case 0 :
                    //no break
                    default:;

                }
                PropertyMapPtr pTableMap( new PropertyMap );
                pTableMap->Insert( PROP_HORI_ORIENT, uno::makeAny( nOrient ) );
                tableProps( pTableMap );
            }
            break;
            case 0x9601: // sprmTDxaLeft
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            break;
            case 0x9602: // sprmTDxaGapHalf
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
            break;
            case 0x3403: // sprmTFCantSplit
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0.5 */
                //row can't break across pages if nIntValue == 1
                PropertyMapPtr pPropMap( new PropertyMap );
                pPropMap->Insert( PROP_IS_SPLIT_ALLOWED, uno::makeAny(sal_Bool( nIntValue == 1 ? sal_False : sal_True ) ));
                insertRowProps(pPropMap);
            }
            break;
            case 0x3404:// sprmTTableHeader
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                // if nIntValue == 1 then the row is a repeated header line
//                PropertyMapPtr pPropMap( new PropertyMap );
//                pPropMap->Insert( , uno::makeAny(sal_Bool( nIntValue == 1 ?  ) ));
//                insertRowProps(pPropMap);
            break;
            case 0xD605: // sprmTTableBorders
            {
                /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
                doctok::Reference<doctok::Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler );
                    pProperties->resolve(*pBorderHandler);
                    //todo: applyBorder()?;
                    cellPropsByCell(0, PropertyMapPtr( pBorderHandler->getProperties()) );
                }
            }
            break;
            default: bRet = false;
        }
    }
    return bRet;
}

}
