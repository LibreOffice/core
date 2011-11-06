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


class ExportXml2007 : public ExportTyp, protected XclExpRoot
{
private:
    ExcDocument*        pExcDoc;

protected:
    RootData*           pExcRoot;

public:
                        ExportXml2007( XclExpRootData& rExpData, SvStream& rStrm );
    virtual             ~ExportXml2007();
    FltError            Write();
};


#endif


