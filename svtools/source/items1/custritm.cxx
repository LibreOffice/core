/*************************************************************************
 *
 *  $RCSfile: custritm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-13 11:15:23 $
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

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <unotools/intlwrapper.hxx>

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SVTOOLS_CUSTRITM_HXX
#include <custritm.hxx>
#endif

//============================================================================
//
//  class CntUnencodedStringItem
//
//============================================================================

DBG_NAME(CntUnencodedStringItem)

//============================================================================
TYPEINIT1_AUTOFACTORY(CntUnencodedStringItem, SfxPoolItem)

//============================================================================
// virtual
int CntUnencodedStringItem::operator ==(const SfxPoolItem & rItem) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rItem.ISA(CntUnencodedStringItem),
               "CntUnencodedStringItem::operator ==(): Bad type");
    return m_aValue
            == SAL_STATIC_CAST(const CntUnencodedStringItem *, &rItem)->
                m_aValue;
}

//============================================================================
// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith) const
{
    DBG_ERROR("CntUnencodedStringItem::Compare(): No international");
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rWith.ISA(CntUnencodedStringItem),
                "CntUnencodedStringItem::Compare(): Bad type");
    switch (m_aValue.CompareTo(static_cast< CntUnencodedStringItem const * >(
                                       &rWith)->
                                   m_aValue))
    {
        case COMPARE_LESS:
            return -1;

        case COMPARE_EQUAL:
            return 0;

        default: // COMPARE_GREATER
            return 1;
    }
}

//============================================================================
// virtual
int CntUnencodedStringItem::Compare(SfxPoolItem const & rWith,
                                    IntlWrapper const & rIntlWrapper)
    const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    DBG_ASSERT(rWith.ISA(CntUnencodedStringItem),
               "CntUnencodedStringItem::Compare(): Bad type");
    return rIntlWrapper.getCollator()->compareString( m_aValue,
        static_cast< CntUnencodedStringItem const * >(&rWith)->m_aValue );
}

//============================================================================
// virtual
SfxItemPresentation
CntUnencodedStringItem::GetPresentation(SfxItemPresentation, SfxMapUnit,
                                        SfxMapUnit, XubString & rText,
                                        const IntlWrapper *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    rText = m_aValue;
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

//============================================================================
// virtual
BOOL CntUnencodedStringItem::QueryValue(com::sun::star::uno::Any& rVal, BYTE)
    const
{
    rVal <<= rtl::OUString(m_aValue);
    return true;
}

//============================================================================
// virtual
BOOL CntUnencodedStringItem::PutValue(const com::sun::star::uno::Any& rVal,
                                         BYTE)
{
    rtl::OUString aTheValue;
    if (rVal >>= aTheValue)
    {
        m_aValue = UniString(aTheValue);
        return true;
    }
    DBG_ERROR("CntUnencodedStringItem::PutValue(): Wrong type");
    return false;
}

//============================================================================
// virtual
SfxPoolItem * CntUnencodedStringItem::Clone(SfxItemPool *) const
{
    DBG_CHKTHIS(CntUnencodedStringItem, 0);
    return new CntUnencodedStringItem(*this);
}

