/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_SCATTR_HXX
#define SC_SCATTR_HXX


#include <bf_svtools/intitem.hxx>

#include <bf_svtools/eitem.hxx>

#include "global.hxx"
namespace binfilter {

//------------------------------------------------------------------------

                                        // Flags fuer durch Merge verdeckte Zellen
                                        // und Control fuer Auto-Filter
#define SC_MF_HOR				1
#define SC_MF_VER				2
#define SC_MF_AUTO				4
#define SC_MF_BUTTON			8
#define SC_MF_SCENARIO			16

#define SC_MF_ALL				31


class EditTextObject;
class SvxBorderLine;

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


    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

            INT16           GetColMerge() const {return nColMerge; }
            INT16           GetRowMerge() const {return nRowMerge; }

            BOOL			IsMerged() const { return nColMerge>1 || nRowMerge>1; }

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

    BOOL	IsHorOverlapped() const		{ return ( GetValue() & SC_MF_HOR ) != 0;  }
    BOOL	IsVerOverlapped() const		{ return ( GetValue() & SC_MF_VER ) != 0;  }
    BOOL	IsOverlapped() const		{ return ( GetValue() & ( SC_MF_HOR | SC_MF_VER ) ) != 0; }

    BOOL	HasAutoFilter() const		{ return ( GetValue() & SC_MF_AUTO ) != 0; }
    BOOL	HasButton() const			{ return ( GetValue() & SC_MF_BUTTON ) != 0; }

    BOOL	IsScenario() const			{ return ( GetValue() & SC_MF_SCENARIO ) != 0; }
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


    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&       Store( SvStream& rStream, USHORT nVer ) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            BOOL            GetProtection() const { return bProtection; }
            BOOL            GetHideFormula() const { return bHideFormula; }
            BOOL            GetHideCell() const { return bHideCell; }
            BOOL            GetHidePrint() const { return bHidePrint; }
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

#define SCR_INVALID		0x01
#define SCR_ALLTABS		0x02
#define SCR_TONEWTAB	0x04

class ScRangeItem : public SfxPoolItem
{
public:
            TYPEINFO();

            inline	ScRangeItem( const USHORT nWhich );
            inline	ScRangeItem( const USHORT   nWhich,
                                 const ScRange& rRange,
                                 const USHORT 	nNewFlags = 0 );
            inline	ScRangeItem( const ScRangeItem& rCpy );

    inline ScRangeItem& operator=( const ScRangeItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 				operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
    virtual USHORT				GetVersion( USHORT nFileVersion ) const;
    virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
    virtual SvStream&			Store( SvStream& rStream, USHORT nVer ) const;

    const ScRange&	GetRange() const 				{ return aRange;  }
    void			SetRange( const ScRange& rNew )	{ aRange = rNew; }

    USHORT			GetFlags() const 				{ return nFlags;  }
    void			SetFlags( USHORT nNew )	 		{ nFlags = nNew; }

private:
    ScRange aRange;
    USHORT	nFlags;
};

inline ScRangeItem::ScRangeItem( const USHORT nWhich )
    :	SfxPoolItem( nWhich ), nFlags( SCR_INVALID ) // == ungueltige Area
{
}

inline ScRangeItem::ScRangeItem( const USHORT	nWhich,
                                 const ScRange& rRange,
                                 const USHORT	nNew )
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

    inline	ScTableListItem( const USHORT nWhich );
            ScTableListItem( const ScTableListItem& rCpy );
            ScTableListItem( const USHORT nWhich, const List& rList );
            ~ScTableListItem();


    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 				operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
    virtual SvStream&			Store( SvStream& rStream, USHORT nVer ) const;

    void	SetTableList( const List& aList );

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

    virtual USHORT			GetVersion( USHORT nFileVersion ) const;

    virtual	bool          QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool          PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    const EditTextObject* GetLeftArea() const		{ return pLeftArea; }
    const EditTextObject* GetCenterArea() const		{ return pCenterArea; }
    const EditTextObject* GetRightArea() const		{ return pRightArea; }

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

    virtual USHORT				GetValueCount() const;
    virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*		Create(SvStream &, USHORT) const;
    virtual USHORT				GetVersion( USHORT nFileVersion ) const;
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

    double GetValue() const		{ return nValue; }

    void SetValue( const double nVal ) { nValue = nVal;}

private:
    double	nValue;
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
