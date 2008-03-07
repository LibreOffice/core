/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chpfld.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:58:19 $
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
#ifndef _CHPFLD_HXX
#define _CHPFLD_HXX

#include "fldbas.hxx"

class SwFrm;
class SwTxtNode;

#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

enum SwChapterFormat
{
    CF_BEGIN,
    CF_NUMBER = CF_BEGIN,       // nur die Kapitelnummer
    CF_TITLE,                   // nur die "Ueberschrift"
    CF_NUM_TITLE,               // Kapitelnummer und "Ueberschrift"
    CF_NUMBER_NOPREPST,         // nur die Kapitelnummer ohne Post/Prefix
    CF_NUM_NOPREPST_TITLE,      // Kapitelnummer ohne Post/Prefix und "Ueberschrift"
    CF_END
};

/*--------------------------------------------------------------------
    Beschreibung: Kapitel
 --------------------------------------------------------------------*/

class SwChapterFieldType : public SwFieldType
{
public:
    SwChapterFieldType();

    virtual SwFieldType*    Copy() const;

};



/*--------------------------------------------------------------------
    Beschreibung: Kapitelnummer
 --------------------------------------------------------------------*/
class SwChapterField : public SwField
{
    friend class SwChapterFieldType;
    BYTE nLevel;
    String sTitle, sNumber, sPre, sPost;
public:
    SwChapterField(SwChapterFieldType*, sal_uInt32 nFmt = 0);

    // --> OD 2008-02-14 #i53420#
//    void ChangeExpansion( const SwFrm*,
//                          const SwTxtNode*,
//                          BOOL bSrchNum = FALSE);
    void ChangeExpansion( const SwFrm*,
                          const SwCntntNode*,
        BOOL bSrchNum = FALSE);
    // <--
    void ChangeExpansion(const SwTxtNode &rNd, BOOL bSrchNum);

    virtual String   Expand() const;
    virtual SwField* Copy() const;

    inline BYTE GetLevel() const;
    inline void SetLevel(BYTE);

    inline const String& GetNumber() const;
    inline const String& GetTitle() const;
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, USHORT nWhich ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, USHORT nWhich );
};

inline BYTE SwChapterField::GetLevel() const    { return nLevel; }
inline void SwChapterField::SetLevel(BYTE nLev) { nLevel = nLev; }
inline const String& SwChapterField::GetNumber() const { return sNumber; }
inline const String& SwChapterField::GetTitle() const { return sTitle; }

#endif // _CHPFLD_HXX
