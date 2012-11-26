/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_LSPCITEM_HXX
#define _SVX_LSPCITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxLineSpacingItem ----------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt den Abstand zwischen den Zeilen.
*/

#define LINE_SPACE_DEFAULT_HEIGHT 200
class EDITENG_DLLPUBLIC SvxLineSpacingItem : public SfxEnumItemInterface
{
    friend SvStream& operator<<( SvStream&, SvxLineSpacingItem& ); //$ ostream

    short nInterLineSpace;
    sal_uInt16 nLineHeight;
    sal_uInt8 nPropLineSpace;
    SvxLineSpace eLineSpace;
    SvxInterLineSpace eInterLineSpace;

public:
    POOLITEM_FACTORY()
    // Der Writer verlaesst sich auf eine Default-Hoehe von 200!
    // Eigentlich wuerde ich alle Werte mit 0 initialisieren, aber wer kann
    // die Folgen beim Writer absehen ?
    // => lieber einen krummen Wert als Default, aber der Programmierer
    // sieht, dass dort etwas besonderes passiert.

    SvxLineSpacingItem( );
    SvxLineSpacingItem( sal_uInt16 nHeight /*= LINE_SPACE_DEFAULT_HEIGHT*/, const sal_uInt16 nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;

    // Methoden zum Abfragen und Aendern
    // Interlinespace wird zur Hoehe addiert.
    inline short GetInterLineSpace() const { return nInterLineSpace; }
    inline void SetInterLineSpace( const short nSpace )
    {
        nInterLineSpace = nSpace;
        eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
    }

    // Bestimmt absolute oder minimale Zeilenhoehe.
    inline sal_uInt16 GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const sal_uInt16 nHeight )
    {
        nLineHeight = nHeight;
        eLineSpace = SVX_LINE_SPACE_MIN;
    }

    // Vergroessert oder verkleinert die Zeilenhoehe.
    sal_uInt8 GetPropLineSpace() const { return nPropLineSpace; }
    inline void SetPropLineSpace( const sal_uInt8 nProp )
    {
        nPropLineSpace = nProp;
        eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
    }

    inline SvxLineSpace &GetLineSpaceRule() { return eLineSpace; }
    inline SvxLineSpace GetLineSpaceRule() const { return eLineSpace; }

    inline SvxInterLineSpace &GetInterLineSpaceRule() { return eInterLineSpace; }
    inline SvxInterLineSpace GetInterLineSpaceRule() const { return eInterLineSpace; }

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16          GetEnumValue() const;
    virtual void            SetEnumValue( sal_uInt16 nNewVal );
};

#endif

