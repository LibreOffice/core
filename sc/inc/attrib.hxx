/*************************************************************************
 *
 *  $RCSfile: attrib.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:47 $
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

#ifndef SC_SCATTR_HXX
#define SC_SCATTR_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------

                                        // Flags fuer durch Merge verdeckte Zellen
                                        // und Control fuer Auto-Filter
#define SC_MF_HOR               1
#define SC_MF_VER               2
#define SC_MF_AUTO              4
#define SC_MF_BUTTON            8
#define SC_MF_SCENARIO          16

#define SC_MF_ALL               31


class EditTextObject;
class SvxBorderLine;

BOOL HasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther );

//------------------------------------------------------------------------

class ScMergeAttr: public SfxPoolItem
{
    INT16       nColMerge;
    INT16       nRowMerge;
public:
                TYPEINFO();
                ScMergeAttr();
                ScMergeAttr( INT16 nCol, INT16 nRow = 0);
                ScMergeAttr( const ScMergeAttr& );
                ~ScMergeAttr();

    virtual String              GetValueText() const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

            INT16           GetColMerge() const {return nColMerge; }
            INT16           GetRowMerge() const {return nRowMerge; }

            BOOL            IsMerged() const { return nColMerge>1 || nRowMerge>1; }

    inline  ScMergeAttr& operator=(const ScMergeAttr& rMerge)
            {
                nColMerge = rMerge.nColMerge;
                nRowMerge = rMerge.nRowMerge;
                return *this;
            }
};

//------------------------------------------------------------------------

class ScMergeFlagAttr: public SfxInt16Item
{
public:
            ScMergeFlagAttr();
            ScMergeFlagAttr(INT16 nFlags);
            ~ScMergeFlagAttr();

    BOOL    IsHorOverlapped() const     { return ( GetValue() & SC_MF_HOR ) != 0;  }
    BOOL    IsVerOverlapped() const     { return ( GetValue() & SC_MF_VER ) != 0;  }
    BOOL    IsOverlapped() const        { return ( GetValue() & ( SC_MF_HOR | SC_MF_VER ) ) != 0; }

    BOOL    HasAutoFilter() const       { return ( GetValue() & SC_MF_AUTO ) != 0; }
    BOOL    HasButton() const           { return ( GetValue() & SC_MF_BUTTON ) != 0; }

    BOOL    IsScenario() const          { return ( GetValue() & SC_MF_SCENARIO ) != 0; }
};

//------------------------------------------------------------------------
class ScProtectionAttr: public SfxPoolItem
{
    BOOL        bProtection;    // Zelle schuetzen
    BOOL        bHideFormula;   // Formel nicht Anzeigen
    BOOL        bHideCell;      // Zelle nicht Anzeigen
    BOOL        bHidePrint;     // Zelle nicht Ausdrucken
public:
                            TYPEINFO();
                            ScProtectionAttr();
                            ScProtectionAttr(   BOOL bProtect,
                                                BOOL bHFormula = FALSE,
                                                BOOL bHCell = FALSE,
                                                BOOL bHPrint = FALSE);
                            ScProtectionAttr( const ScProtectionAttr& );
                            ~ScProtectionAttr();

    virtual String              GetValueText() const;
    virtual SfxItemPresentation GetPresentation(
                                    SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String& rText,
                                    const International* pIntl = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            BOOL            GetProtection() const { return bProtection; }
            BOOL            SetProtection( BOOL bProtect);
            BOOL            GetHideFormula() const { return bHideFormula; }
            BOOL            SetHideFormula( BOOL bHFormula);
            BOOL            GetHideCell() const { return bHideCell; }
            BOOL            SetHideCell( BOOL bHCell);
            BOOL            GetHidePrint() const { return bHidePrint; }
            BOOL            SetHidePrint( BOOL bHPrint);
    inline  ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
            {
                bProtection = rProtection.bProtection;
                bHideFormula = rProtection.bHideFormula;
                bHideCell = rProtection.bHideCell;
                bHidePrint = rProtection.bHidePrint;
                return *this;
            }
};


//----------------------------------------------------------------------------
// ScRangeItem: verwaltet einen Tabellenbereich

#define SCR_INVALID     0x01
#define SCR_ALLTABS     0x02
#define SCR_TONEWTAB    0x04

class ScRangeItem : public SfxPoolItem
{
public:
            TYPEINFO();

            inline  ScRangeItem( const USHORT nWhich );
            inline  ScRangeItem( const USHORT   nWhich,
                                 const ScRange& rRange,
                                 const USHORT   nNewFlags = 0 );
            inline  ScRangeItem( const ScRangeItem& rCpy );

    inline ScRangeItem& operator=( const ScRangeItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual void                Record( SfxArguments &rArgs ) const;
    virtual SfxArgumentError    Construct( USHORT nId, const SfxArguments &rArgs );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String &rText,
                                                 const International* pIntl = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT              GetVersion( USHORT nFileVersion ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT) const;
    virtual SvStream&           Store( SvStream& rStream, USHORT nVer ) const;

    const ScRange&  GetRange() const                { return aRange;  }
    void            SetRange( const ScRange& rNew ) { aRange = rNew; }

    USHORT          GetFlags() const                { return nFlags;  }
    void            SetFlags( USHORT nNew )         { nFlags = nNew; }

private:
    ScRange aRange;
    USHORT  nFlags;
};

inline ScRangeItem::ScRangeItem( const USHORT nWhich )
    :   SfxPoolItem( nWhich ), nFlags( SCR_INVALID ) // == ungueltige Area
{
}

inline ScRangeItem::ScRangeItem( const USHORT   nWhich,
                                 const ScRange& rRange,
                                 const USHORT   nNew )
    : SfxPoolItem( nWhich ), aRange( rRange ), nFlags( nNew )
{
}

inline ScRangeItem::ScRangeItem( const ScRangeItem& rCpy )
    : SfxPoolItem( rCpy.Which() ), aRange( rCpy.aRange ), nFlags( rCpy.nFlags )
{}

inline ScRangeItem& ScRangeItem::operator=( const ScRangeItem &rCpy )
{
    aRange = rCpy.aRange;
    return *this;
}

//----------------------------------------------------------------------------
// ScTableListItem: verwaltet eine Liste von Tabellen
//----------------------------------------------------------------------------
class ScTableListItem : public SfxPoolItem
{
public:
    TYPEINFO();

    inline  ScTableListItem( const USHORT nWhich );
            ScTableListItem( const ScTableListItem& rCpy );
            ScTableListItem( const USHORT nWhich, const List& rList );
            ~ScTableListItem();

    ScTableListItem& operator=( const ScTableListItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual void                Record( SfxArguments &rArgs ) const;
    virtual SfxArgumentError    Construct( USHORT nId, const SfxArguments &rArgs );
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String &rText,
                                                 const International* pIntl = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT) const;
    virtual SvStream&           Store( SvStream& rStream, USHORT nVer ) const;

    BOOL    GetTableList( List& aList ) const;
    void    SetTableList( const List& aList );

public:
    USHORT  nCount;
    USHORT* pTabArr;
};

inline ScTableListItem::ScTableListItem( const USHORT nWhich )
    : SfxPoolItem(nWhich), nCount(0), pTabArr(NULL)
{}

//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

#define SC_HF_LEFTAREA   1
#define SC_HF_CENTERAREA 2
#define SC_HF_RIGHTAREA  3

class ScPageHFItem : public SfxPoolItem
{
    EditTextObject* pLeftArea;
    EditTextObject* pCenterArea;
    EditTextObject* pRightArea;

public:
                TYPEINFO();
                ScPageHFItem( USHORT nWhich );
                ScPageHFItem( const ScPageHFItem& rItem );
                ~ScPageHFItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

    virtual USHORT          GetVersion( USHORT nFileVersion ) const;

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual BOOL            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const EditTextObject* GetLeftArea() const       { return pLeftArea; }
    const EditTextObject* GetCenterArea() const     { return pCenterArea; }
    const EditTextObject* GetRightArea() const      { return pRightArea; }

    void SetLeftArea( const EditTextObject& rNew );
    void SetCenterArea( const EditTextObject& rNew );
    void SetRightArea( const EditTextObject& rNew );

    //Set mit Uebereignung der Pointer, nArea siehe defines oben
    void SetArea( EditTextObject *pNew, int nArea );
};


//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

class ScViewObjectModeItem: public SfxEnumItem
{
public:
                TYPEINFO();

                ScViewObjectModeItem( USHORT nWhich );
                ScViewObjectModeItem( USHORT nWhich, ScVObjMode eMode );
                ~ScViewObjectModeItem();

    virtual USHORT              GetValueCount() const;
    virtual String              GetValueText( USHORT nVal ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*        Create(SvStream &, USHORT) const;
    virtual USHORT              GetVersion( USHORT nFileVersion ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String& rText,
                                                 const International* pIntl = 0 ) const;
};

//----------------------------------------------------------------------------
//

class ScDoubleItem : public SfxPoolItem
{
public:
                TYPEINFO();
                ScDoubleItem( USHORT nWhich, double nVal=0 );
                ScDoubleItem( const ScDoubleItem& rItem );
                ~ScDoubleItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

    double GetValue() const     { return nValue; }

    void SetValue( const double nVal ) { nValue = nVal;}

private:
    double  nValue;
};


#endif

