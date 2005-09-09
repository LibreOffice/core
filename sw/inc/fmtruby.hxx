/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmtruby.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:53:02 $
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
#ifndef _FMTRUBY_HXX
#define _FMTRUBY_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

class SwTxtRuby;

class SwFmtRuby : public SfxPoolItem
{
    friend class SwTxtRuby;

    String sRubyTxt;                    // the ruby txt
    String sCharFmtName;                // name of the charformat
    SwTxtRuby* pTxtAttr;                // the TextAttribut
    USHORT nCharFmtId;                  // PoolId of the charformat
    USHORT nPosition;                   // Position of the Ruby-Character
    USHORT nAdjustment;                 // specific adjustment of the Ruby-Ch.

public:
    SwFmtRuby( const String& rRubyTxt );
    SwFmtRuby( const SwFmtRuby& rAttr );
    virtual ~SwFmtRuby();

    SwFmtRuby& operator=( const SwFmtRuby& rAttr );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue( const com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );


    const SwTxtRuby* GetTxtRuby() const         { return pTxtAttr; }
    SwTxtRuby* GetTxtRuby()                     { return pTxtAttr; }

    const String& GetText() const               { return sRubyTxt; }
    void SetText( const String& rTxt )          { sRubyTxt = rTxt; }

    const String& GetCharFmtName() const        { return sCharFmtName; }
    void SetCharFmtName( const String& rNm )    { sCharFmtName = rNm; }

    USHORT GetCharFmtId() const                 { return nCharFmtId; }
    void SetCharFmtId( USHORT nNew )            { nCharFmtId = nNew; }

    USHORT GetPosition() const                  { return nPosition; }
    void SetPosition( USHORT nNew )             { nPosition = nNew; }

    USHORT GetAdjustment() const                { return nAdjustment; }
    void SetAdjustment( USHORT nNew )           { nAdjustment = nNew; }
};


#endif

