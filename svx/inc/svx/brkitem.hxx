/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brkitem.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:39:36 $
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
#ifndef _SVX_BRKITEM_HXX
#define _SVX_BRKITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace rtl
{
    class OUString;
}

// class SvxFmtBreakItem -------------------------------------------------

/*
    [Beschreibung]
    Dieses Item beschreibt ein Umbruch-Attribut
    Automatisch?, Seiten- oder Spaltenumbruch, davor oder danach?
*/
#define FMTBREAK_NOAUTO ((USHORT)0x0001)

class SVX_DLLPUBLIC SvxFmtBreakItem : public SfxEnumItem
{
public:
    TYPEINFO();

    inline SvxFmtBreakItem( const SvxBreak eBrk = SVX_BREAK_NONE,
#if SUPD<372
                            const BOOL bAuto = FALSE,
#endif
                            const USHORT nWhich = ITEMID_FMTBREAK );
    inline SvxFmtBreakItem( const SvxFmtBreakItem& rBreak );
    inline SvxFmtBreakItem& operator=( const SvxFmtBreakItem& rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual String           GetValueTextByPos( USHORT nPos ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SvStream&        Store( SvStream& , USHORT nItemVersion ) const;
    virtual USHORT           GetVersion( USHORT nFileVersion ) const;
    virtual SfxPoolItem*     Create( SvStream&, USHORT ) const;
    virtual USHORT           GetValueCount() const;

    // MS VC4.0 kommt durcheinander
    void             SetValue( USHORT nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    const SvxBreak           GetBreak() const { return SvxBreak( GetValue() ); }
    void                     SetBreak( const SvxBreak eNew )
                                { SetValue( (USHORT)eNew ); }
#if SUPD<372
    BOOL                     IsAuto() const { return bAuto; }
private:
    BOOL bAuto;     //Fuer automatische Breaks, wirkt auf das Verhalten bei
                    //Backspace und Delete. Wird nie veraendert, nicht const
                    //wg. dem Zuweisungsoperator.
    SVX_DLLPRIVATE void SetAuto( const BOOL bNew ) { bAuto = bNew; }
#endif
};

#if SUPD<372

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxBreak eBreak,
                                         const BOOL bA,
                                         const USHORT nWhich ) :
    SfxEnumItem( nWhich, (USHORT)eBreak ),
    bAuto( bA )
{}

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxFmtBreakItem& rBreak ) :
    SfxEnumItem( rBreak ),
    bAuto( rBreak.IsAuto() )
{}

inline SvxFmtBreakItem& SvxFmtBreakItem::operator=(
    const SvxFmtBreakItem& rBreak )
{
    SetValue( rBreak.GetValue() );
    SetAuto( rBreak.IsAuto() );
    return *this;
}

#else

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxBreak eBreak,
                                         const USHORT _nWhich ) :
    SfxEnumItem( _nWhich, (USHORT)eBreak )
{}

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxFmtBreakItem& rBreak ) :
    SfxEnumItem( rBreak )
{}

inline SvxFmtBreakItem& SvxFmtBreakItem::operator=(
    const SvxFmtBreakItem& rBreak )
{
    SetValue( rBreak.GetValue() );
    return *this;
}

#endif

#endif

