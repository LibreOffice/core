/*************************************************************************
 *
 *  $RCSfile: rulritem.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:58 $
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
#ifndef _SVX_RULRITEM_HXX
#define _SVX_RULRITEM_HXX

// include ---------------------------------------------------------------


#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

// class SvxLongLRSpaceItem ----------------------------------------------

class SvxLongLRSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft oder der neg. Erstzeileneinzug
    long    lRight;        // der unproblematische rechte Rand

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongLRSpaceItem(long lLeft, long lRight, USHORT nId);
    SvxLongLRSpaceItem(const SvxLongLRSpaceItem &);

    long    GetLeft() const { return lLeft; }
    long    GetRight() const { return lRight; }
    void    SetLeft(long lArgLeft) {lLeft=lArgLeft;}
    void    SetRight(long lArgRight) {lRight=lArgRight;}
};

// class SvxLongULSpaceItem ----------------------------------------------

class SvxLongULSpaceItem : public SfxPoolItem
{
    long    lLeft;         // nLeft oder der neg. Erstzeileneinzug
    long    lRight;        // der unproblematische rechte Rand

  protected:

    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &); // n.i.

public:
    TYPEINFO();
    SvxLongULSpaceItem(long lUpper, long lLower, USHORT nId);
    SvxLongULSpaceItem(const SvxLongULSpaceItem &);

    long    GetUpper() const { return lLeft; }
    long    GetLower() const { return lRight; }
    void    SetUpper(long lArgLeft) {lLeft=lArgLeft;}
    void    SetLower(long lArgRight) {lRight=lArgRight;}
};

// class SvxPagePosSizeItem ----------------------------------------------

class SvxPagePosSizeItem : public SfxPoolItem
{
    Point aPos;
    long lWidth;
    long lHeight;
protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

private:
    const SvxPagePosSizeItem& operator=(const SvxPagePosSizeItem &); // n.i.
public:
    TYPEINFO();
    SvxPagePosSizeItem(const Point &rPos, long lWidth, long lHeight);
    SvxPagePosSizeItem(const SvxPagePosSizeItem &);

    const Point &GetPos() const { return aPos; }
    long    GetWidth() const { return lWidth; }
    long    GetHeight() const { return lHeight; }
};

// struct SvxColumnDescription -------------------------------------------

struct SvxColumnDescription
{
    USHORT nStart;                     /* Spaltenbeginn */
    USHORT nEnd;                       /* Spaltenende */
    BOOL   bVisible;                   /* Sichtbarkeit */
    SvxColumnDescription():
        nStart(0), nEnd(0), bVisible(TRUE) {}
    SvxColumnDescription(const SvxColumnDescription &rCopy) :
        nStart(rCopy.nStart), nEnd(rCopy.nEnd), bVisible(rCopy.bVisible) {}
    SvxColumnDescription(USHORT start, USHORT end, BOOL bVis = TRUE):
        nStart(start), nEnd(end), bVisible(bVis) {}
    int operator==(const SvxColumnDescription &rCmp) const {
        return nStart == rCmp.nStart &&
            bVisible == rCmp.bVisible &&
            nEnd == rCmp.nEnd;
    }
    int operator!=(const SvxColumnDescription &rCmp) const {
        return !operator==(rCmp);
    }
    USHORT GetWidth() const { return nEnd - nStart; }
};

// class SvxColumnItem ---------------------------------------------------

typedef SvPtrarr SvxColumns;

class SvxColumnItem : public SfxPoolItem
{
    SvxColumns aColumns;// Spaltenarray
    long    nLeft,      // Linker Rand bei Tabelle
           nRight;      // Rechter Rand bei Tabelle; bei Spalten immer gleich
                        // zum umgebenden Rahmen
    USHORT nActColumn;  // die aktuelle Spalte
    BOOL    bTable;     // Tabelle?
    BOOL    bOrtho;     // Gleichverteilte Spalten

    void DeleteAndDestroyColumns();

protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
public:
    TYPEINFO();
    // rechter Rand des umgebenden Rahmens
    // nLeft, nRight jeweils der Abstand zum umgebenden Rahmen
    SvxColumnItem(USHORT nAct = 0); // Spalten
    SvxColumnItem(USHORT nActCol,
                  USHORT nLeft, USHORT nRight = 0); // Tabelle mit Raendern
    SvxColumnItem(const SvxColumnItem &);
    ~SvxColumnItem();

    const SvxColumnItem &operator=(const SvxColumnItem &);

    USHORT Count() const { return aColumns.Count(); }
    SvxColumnDescription &operator[](USHORT i)
        { return *(SvxColumnDescription*)aColumns[i]; }
    const SvxColumnDescription &operator[](USHORT i) const
        { return *(SvxColumnDescription*)aColumns[i]; }
    void Insert(const SvxColumnDescription &rDesc, USHORT nPos) {
        SvxColumnDescription* pDesc = new SvxColumnDescription(rDesc);
        aColumns.Insert(pDesc, nPos);
    }
    void   Append(const SvxColumnDescription &rDesc) { Insert(rDesc, Count()); }
    void   SetLeft(long left) { nLeft = left; }
    void   SetRight(long right) { nRight = right; }
    void   SetActColumn(USHORT nCol) { nActColumn = nCol; }

    USHORT GetActColumn() const { return nActColumn; }
    BOOL   IsFirstAct() const { return nActColumn == 0; }
    BOOL   IsLastAct() const { return nActColumn == Count()-1; }
    long GetLeft() { return nLeft; }
    long GetRight() { return nRight; }

    BOOL   IsTable() const { return bTable; }

    BOOL   CalcOrtho() const;
    void   SetOrtho(BOOL bVal) { bOrtho = bVal; }
    BOOL   IsOrtho () const { return FALSE ; }

    BOOL IsConsistent() const  { return nActColumn < aColumns.Count(); }
    long   GetVisibleRight() const;// rechter sichtbare Rand der aktuellen Spalte
};

// class SvxObjectItem ---------------------------------------------------

class SvxObjectItem : public SfxPoolItem
{
private:
    long   nStartX;                    /* Beginn in X-Richtung */
    long   nEndX;                      /* Ende in X-Richtung */
    long   nStartY;                    /* Beginn in Y-Richtung */
    long   nEndY;                      /* Ende in Y-Richtung */
    BOOL   bLimits;                    /* Grenzwertkontrolle durch die Applikation */
protected:
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual String           GetValueText() const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const International * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
private:
    const SvxObjectItem &operator=(const SvxObjectItem &); // n.i.
public:
    TYPEINFO();
    SvxObjectItem(long nStartX, long nEndX,
                  long nStartY, long nEndY,
                  BOOL bLimits = FALSE);
    SvxObjectItem(const SvxObjectItem &);

    BOOL   HasLimits() const { return bLimits; }

    long   GetStartX() const { return nStartX; }
    long   GetEndX() const { return nEndX; }
    long   GetStartY() const { return nStartY; }
    long   GetEndY() const { return nEndY; }

    void   SetStartX(long l) { nStartX = l; }
    void   SetEndX(long l) { nEndX = l; }
    void   SetStartY(long l) { nStartY = l; }
    void   SetEndY(long l) { nEndY = l; }
};


#endif

