/*************************************************************************
 *
 *  $RCSfile: PageMasterPropMapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-18 11:30:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERPROPMAPPER_HXX
#include "PageMasterPropMapper.hxx"
#endif

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


//______________________________________________________________________________

inline void lcl_RemoveState( XMLPropertyState* pState )
{
    pState->mnIndex = -1;
    pState->maValue.clear();
}

inline sal_Bool lcl_HasSameLineWidth( const table::BorderLine& rLine1, const table::BorderLine& rLine2 )
{
    return  (rLine1.InnerLineWidth == rLine2.InnerLineWidth) &&
            (rLine1.OuterLineWidth == rLine2.OuterLineWidth) &&
            (rLine1.LineDistance == rLine2.LineDistance);
}

inline sal_Bool operator==( const table::BorderLine& rLine1, const table::BorderLine& rLine2 )
{
    return  (rLine1.Color == rLine2.Color) &&
            lcl_HasSameLineWidth( rLine1, rLine2 );
}

//______________________________________________________________________________
XMLPageMasterPropSetMapper::XMLPageMasterPropSetMapper(
        const XMLPropertyMapEntry* pEntries,
        const UniReference< XMLPropertyHandlerFactory >& rFactory ) :
    XMLPropertySetMapper( pEntries, rFactory )
{
}

XMLPageMasterPropSetMapper::~XMLPageMasterPropSetMapper()
{
}

void XMLPageMasterPropSetMapper::ContextFilter(
        ::std::vector< XMLPropertyState >& rPropState,
        Reference< XPropertySet > rPropSet ) const
{
    XMLPropertyState*   pPMBorderAll            = NULL;
    XMLPropertyState*   pPMBorderTop            = NULL;
    XMLPropertyState*   pPMBorderBottom         = NULL;
    XMLPropertyState*   pPMBorderLeft           = NULL;
    XMLPropertyState*   pPMBorderRight          = NULL;

    XMLPropertyState*   pPMBorderWidthAll       = NULL;
    XMLPropertyState*   pPMBorderWidthTop       = NULL;
    XMLPropertyState*   pPMBorderWidthBottom    = NULL;
    XMLPropertyState*   pPMBorderWidthLeft      = NULL;
    XMLPropertyState*   pPMBorderWidthRight     = NULL;

    XMLPropertyState*   pPMPaddingAll           = NULL;
    XMLPropertyState*   pPMPaddingTop           = NULL;
    XMLPropertyState*   pPMPaddingBottom        = NULL;
    XMLPropertyState*   pPMPaddingLeft          = NULL;
    XMLPropertyState*   pPMPaddingRight         = NULL;

    XMLPropertyState*   pPMBackgroundColor      = NULL;
    XMLPropertyState*   pPMIsBackgroundTrans    = NULL;

    for( ::std::vector< XMLPropertyState >::iterator pProp = rPropState.begin(); pProp != rPropState.end(); pProp++ )
    {
        switch( GetEntryContextId( pProp->mnIndex ) )
        {
            case CTF_PM_BORDERALL:          pPMBorderAll            = pProp;    break;
            case CTF_PM_BORDERTOP:          pPMBorderTop            = pProp;    break;
            case CTF_PM_BORDERBOTTOM:       pPMBorderBottom         = pProp;    break;
            case CTF_PM_BORDERLEFT:         pPMBorderLeft           = pProp;    break;
            case CTF_PM_BORDERRIGHT:        pPMBorderRight          = pProp;    break;
            case CTF_PM_BORDERWIDTHALL:     pPMBorderWidthAll       = pProp;    break;
            case CTF_PM_BORDERWIDTHTOP:     pPMBorderWidthTop       = pProp;    break;
            case CTF_PM_BORDERWIDTHBOTTOM:  pPMBorderWidthBottom    = pProp;    break;
            case CTF_PM_BORDERWIDTHLEFT:    pPMBorderWidthLeft      = pProp;    break;
            case CTF_PM_BORDERWIDTHRIGHT:   pPMBorderWidthRight     = pProp;    break;
            case CTF_PM_PADDINGALL:         pPMPaddingAll           = pProp;    break;
            case CTF_PM_PADDINGTOP:         pPMPaddingTop           = pProp;    break;
            case CTF_PM_PADDINGBOTTOM:      pPMPaddingBottom        = pProp;    break;
            case CTF_PM_PADDINGLEFT:        pPMPaddingLeft          = pProp;    break;
            case CTF_PM_PADDINGRIGHT:       pPMPaddingRight         = pProp;    break;
            case CTF_PM_BACKGROUNDCOLOR:    pPMBackgroundColor      = pProp;    break;
            case CTF_PM_ISBACKGROUNDTRANS:  pPMIsBackgroundTrans    = pProp;    break;
        }
    }

    if( pPMBorderAll )
    {
        if( pPMBorderTop && pPMBorderBottom && pPMBorderLeft && pPMBorderRight )
        {
            table::BorderLine aLineTop, aLineBottom, aLineLeft, aLineRight;

            pPMBorderTop->maValue >>= aLineTop;
            pPMBorderBottom->maValue >>= aLineBottom;
            pPMBorderLeft->maValue >>= aLineLeft;
            pPMBorderRight->maValue >>= aLineRight;

            if( (aLineTop == aLineBottom) && (aLineBottom == aLineLeft) && (aLineLeft == aLineRight) )
            {
                lcl_RemoveState( pPMBorderTop );
                lcl_RemoveState( pPMBorderBottom );
                lcl_RemoveState( pPMBorderLeft );
                lcl_RemoveState( pPMBorderRight );
            }
            else
                lcl_RemoveState( pPMBorderAll );
        }
        else
            lcl_RemoveState( pPMBorderAll );
    }

    if( pPMBorderWidthAll )
    {
        if( pPMBorderWidthTop && pPMBorderWidthBottom && pPMBorderWidthLeft && pPMBorderWidthRight )
        {
            table::BorderLine aLineTop, aLineBottom, aLineLeft, aLineRight;

            pPMBorderWidthTop->maValue >>= aLineTop;
            pPMBorderWidthBottom->maValue >>= aLineBottom;
            pPMBorderWidthLeft->maValue >>= aLineLeft;
            pPMBorderWidthRight->maValue >>= aLineRight;

            if( lcl_HasSameLineWidth( aLineTop, aLineBottom ) &&
                lcl_HasSameLineWidth( aLineBottom, aLineLeft ) &&
                lcl_HasSameLineWidth( aLineLeft, aLineRight ) )
            {
                lcl_RemoveState( pPMBorderWidthTop );
                lcl_RemoveState( pPMBorderWidthBottom );
                lcl_RemoveState( pPMBorderWidthLeft );
                lcl_RemoveState( pPMBorderWidthRight );
            }
            else
                lcl_RemoveState( pPMBorderWidthAll );
        }
        else
            lcl_RemoveState( pPMBorderWidthAll );
    }

    if( pPMPaddingAll )
    {
        if( pPMPaddingTop && pPMPaddingBottom && pPMPaddingLeft && pPMPaddingRight )
        {
            sal_Int32 nTop, nBottom, nLeft, nRight;

            pPMPaddingTop->maValue >>= nTop;
            pPMPaddingBottom->maValue >>= nBottom;
            pPMPaddingLeft->maValue >>= nLeft;
            pPMPaddingRight->maValue >>= nRight;

            if( (nTop == nBottom) && (nBottom == nLeft) && (nLeft == nRight) )
            {
                lcl_RemoveState( pPMPaddingTop );
                lcl_RemoveState( pPMPaddingBottom );
                lcl_RemoveState( pPMPaddingLeft );
                lcl_RemoveState( pPMPaddingRight );
            }
            else
                lcl_RemoveState( pPMPaddingAll );
        }
        else
            lcl_RemoveState( pPMPaddingAll );
    }

    if( pPMBackgroundColor && pPMIsBackgroundTrans )
    {
        sal_Bool bTransparent;

        if( (pPMIsBackgroundTrans->maValue >>= bTransparent) && bTransparent )
            lcl_RemoveState( pPMBackgroundColor );
        else
            lcl_RemoveState( pPMIsBackgroundTrans );
    }
}

