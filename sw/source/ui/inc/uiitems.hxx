/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uiitems.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:11:24 $
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
#ifndef _UIITEMS_HXX
#define _UIITEMS_HXX

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef INCLUDED_SW_CMDID_H
#include "cmdid.h"
#endif
#ifndef _PAGEDESC_HXX
#include "pagedesc.hxx"
#endif

class SwNumRule;
class IntlWrapper;

/*--------------------------------------------------------------------
    Beschreibung: Container fuer FootNote
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwPageFtnInfoItem : public SfxPoolItem
{
    SwPageFtnInfo aFtnInfo;

public:

    SwPageFtnInfoItem(const USHORT nId, SwPageFtnInfo& rInfo);
    SwPageFtnInfoItem(const SwPageFtnInfoItem& rItem );
    ~SwPageFtnInfoItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    SwPageFtnInfo& GetPageFtnInfo()             { return aFtnInfo; }
    const SwPageFtnInfo& GetPageFtnInfo() const { return aFtnInfo; }
    void SetPageFtnInfo(SwPageFtnInfo& rInf)    { aFtnInfo = rInf; }
};

/*******************************************************************/

class SW_DLLPUBLIC SwPtrItem : public SfxPoolItem
{
    void* pMisc;

public:
    SwPtrItem( const USHORT nId = FN_PARAM_GRF_DIALOG, void* pPtr = 0);
    SwPtrItem( const SwPtrItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void    SetValue(void * pPtr)   { pMisc= pPtr; }
    void*   GetValue() const        { return pMisc; }
};

/*******************************************************************/

class SW_DLLPUBLIC SwUINumRuleItem : public SfxPoolItem
{
    SwNumRule* pRule;

public:
    SwUINumRuleItem( const SwNumRule& rRule, const USHORT = FN_PARAM_ACT_NUMBER);
    SwUINumRuleItem( const SwUINumRuleItem& rItem );
    virtual ~SwUINumRuleItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual BOOL             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const SwNumRule* GetNumRule() const         { return pRule; }
          SwNumRule* GetNumRule()               { return pRule; }
};

/* -----------------17.06.98 17:40-------------------
 *
 * --------------------------------------------------*/
class SwBackgroundDestinationItem : public SfxUInt16Item
{
public:
    SwBackgroundDestinationItem(USHORT  nWhich, USHORT nValue);

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
};

#endif // _UIITEMS_HXX
