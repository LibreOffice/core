/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmthdft.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:51:04 $
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
#ifndef _FMTHDFT_HXX
#define _FMTHDFT_HXX

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _FORMAT_HXX //autogen
#include <format.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif

class SwFrmFmt;
class IntlWrapper;


//Kopfzeile, fuer Seitenformate
//Client von FrmFmt das den Header beschreibt.

class SwFmtHeader: public SfxPoolItem, public SwClient
{
    BOOL bActive;       //Nur zur Steuerung (Erzeugung des Inhaltes)

public:
    SwFmtHeader( BOOL bOn = FALSE );
    SwFmtHeader( SwFrmFmt *pHeaderFmt );
    SwFmtHeader( const SwFmtHeader &rCpy );
    ~SwFmtHeader();
    SwFmtHeader& operator=( const SwFmtHeader &rCpy );

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetHeaderFmt() const { return (SwFrmFmt*)pRegisteredIn; }
          SwFrmFmt *GetHeaderFmt()       { return (SwFrmFmt*)pRegisteredIn; }

    BOOL IsActive() const { return bActive; }
    void SetActive( BOOL bNew = TRUE ) { bActive = bNew; }
};

//Fusszeile, fuer Seitenformate
//Client von FrmFmt das den Footer beschreibt.

class SwFmtFooter: public SfxPoolItem, public SwClient
{
    BOOL bActive;       //Nur zur Steuerung (Erzeugung des Inhaltes)

public:
    SwFmtFooter( BOOL bOn = FALSE );
    SwFmtFooter( SwFrmFmt *pFooterFmt );
    SwFmtFooter( const SwFmtFooter &rCpy );
    ~SwFmtFooter();
    SwFmtFooter& operator=( const SwFmtFooter &rCpy );

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    const SwFrmFmt *GetFooterFmt() const { return (SwFrmFmt*)pRegisteredIn; }
          SwFrmFmt *GetFooterFmt()       { return (SwFrmFmt*)pRegisteredIn; }

    BOOL IsActive() const { return bActive; }
    void SetActive( BOOL bNew = TRUE ) { bActive = bNew; }
};

inline const SwFmtHeader &SwAttrSet::GetHeader(BOOL bInP) const
    { return (const SwFmtHeader&)Get( RES_HEADER,bInP); }
inline const SwFmtFooter &SwAttrSet::GetFooter(BOOL bInP) const
    { return (const SwFmtFooter&)Get( RES_FOOTER,bInP); }

inline const SwFmtHeader &SwFmt::GetHeader(BOOL bInP) const
    { return aSet.GetHeader(bInP); }
inline const SwFmtFooter &SwFmt::GetFooter(BOOL bInP) const
    { return aSet.GetFooter(bInP); }

#endif

