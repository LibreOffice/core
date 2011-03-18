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

#ifndef SC_EXP_OP_HXX
#define SC_EXP_OP_HXX

#include "filter.hxx"
#include "root.hxx"
#include "xeroot.hxx"


class ScDocument;
class ScPatternAttr;
class ScFormulaCell;
class ExcDocument;
class SotStorage;


class ExportTyp
{
protected:
    SvStream&           aOut;           // Ausgabe-Stream
    ScDocument*         pD;             // Dokument
    CharSet             eZielChar;      // Ziel-Zeichensatz
public:
                        ExportTyp( SvStream& aStream, ScDocument* pDoc, CharSet eDest ):
                            aOut( aStream )
                        {
                            eZielChar = eDest;
                            pD = pDoc;
                        }

    virtual FltError    Write() = 0;
};



#if ENABLE_LOTUS123_EXPORT
class ExportWK1 : public ExportTyp
{
private:
    sal_uInt8               GenFormByte( const ScPatternAttr& );
    void                Bof();
    void                Eof();
    void                Calcmode();
    void                Calcorder();
    void                Split();
    void                Sync();
    void                Dimensions();
    void                Window1();
    void                Colw();
    void                Blank( const sal_uInt16 nC, const sal_uInt16 nR, const ScPatternAttr& );
    void                Number( const sal_uInt16 nC, const sal_uInt16 nR, const double f, const ScPatternAttr& );
    void                Label( const sal_uInt16 nC, const sal_uInt16 nR, const String&, const ScPatternAttr& );
    void                Formula( const sal_uInt16 nC, const sal_uInt16 nR, const ScFormulaCell*, const ScPatternAttr& );
    void                Protect();
    void                Footer();
    void                Header();
    void                Margins();
    void                Labelfmt();
    void                Calccount();
    void                Cursorw12();
    void                WKString( const sal_uInt16 nC, const sal_uInt16 nR, const ScFormulaCell*, const ScPatternAttr& );
    void                Snrange();
    void                Hidcol();
    void                Cpi();
public:

    static  const sal_uInt16    WK1MAXCOL;
    static  const sal_uInt16    WK1MAXROW;

    inline              ExportWK1( SvStream& r, ScDocument* p, CharSet e ) :
                            ExportTyp( r, p, e ) {};

    FltError            Write();
};
#endif



class ExportBiff5 : public ExportTyp, protected XclExpRoot
{
private:
    ExcDocument*        pExcDoc;

protected:
    RootData*           pExcRoot;

public:
                        ExportBiff5( XclExpRootData& rExpData, SvStream& rStrm );
    virtual             ~ExportBiff5();
    FltError            Write();
};




class ExportBiff8 : public ExportBiff5
{
public:
                        ExportBiff8( XclExpRootData& rExpData, SvStream& rStrm );
    virtual             ~ExportBiff8();
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
