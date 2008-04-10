/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bookmrk.hxx,v $
 * $Revision: 1.10 $
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
#ifndef _BOOKMRK_HXX
#define _BOOKMRK_HXX

#include "hintids.hxx"      //die Ids der Attribute, vor macitem damit die
                            //die Attribut richtig angezogen werden.
#include <svtools/macitem.hxx>

#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#include <IDocumentBookmarkAccess.hxx>
#include <calbck.hxx>
#include <pam.hxx>

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif


struct SwPosition;  // fwd Decl. wg. UI

class SwBookmark : public SwModify
{
    SwPosition *pPos1, *pPos2;  // wird im CTOR gesetzt, im DTOR geloescht
                                // pPos1 is always != 0, pPos2 may be 0
    SwServerObjectRef refObj;   // falls DataServer -> Pointer gesetzt

protected:
    String      aName;
    String      aShortName;
    KeyCode     aCode;
    IDocumentBookmarkAccess::BookmarkType eMarkType;

    SwBookmark( const SwPosition& aPos,
                const KeyCode& rCode,
                const String& rName, const String& rShortName);

public:
    TYPEINFO();

    SwBookmark( const SwPosition& aPos );
    // --> OD 2007-09-26 #i81002#
    SwBookmark( const SwPaM& aPaM,
                const KeyCode& rCode,
                const String& rName, const String& rShortName);
    // <--

    // Beim Loeschen von Text werden Bookmarks mitgeloescht!
    virtual ~SwBookmark();

    // --> OD 2007-10-10 #i81002#
    // made virtual and thus no longer inline
    virtual const SwPosition& GetBookmarkPos() const;
    virtual const SwPosition* GetOtherBookmarkPos() const;
    // <--

    // nicht undofaehig
    const String& GetName() const { return aName; }
    // nicht undofaehig
    const String& GetShortName() const { return aShortName; }
    // nicht undofaehig
    const KeyCode& GetKeyCode() const { return aCode; }

    // Vergleiche auf Basis der Dokumentposition
    BOOL operator < (const SwBookmark &) const;
    BOOL operator ==(const SwBookmark &) const;
    // falls man wirklich auf gleiche Position abfragen will.
    BOOL IsEqualPos( const SwBookmark &rBM ) const;

    BOOL IsBookMark() const    { return IDocumentBookmarkAccess::BOOKMARK == eMarkType; }
//    // --> OD 2007-10-17 #TESTING#
//    BOOL IsBookMark() const
//    {
//        return IDocumentBookmarkAccess::BOOKMARK == eMarkType ||
//               IsCrossRefMark();
//    }
//    // <--
    BOOL IsMark() const         { return IDocumentBookmarkAccess::MARK == eMarkType; }
    BOOL IsUNOMark() const      { return IDocumentBookmarkAccess::UNO_BOOKMARK == eMarkType; }
    // --> OD 2007-10-11 #i81002# - bookmark type for cross-references
    BOOL IsCrossRefMark() const { return IDocumentBookmarkAccess::CROSSREF_BOOKMARK == eMarkType; }
    // <--
    void SetType( IDocumentBookmarkAccess::BookmarkType eNewType )  { eMarkType = eNewType; }
    IDocumentBookmarkAccess::BookmarkType GetType() const   { return eMarkType; }

        // Daten Server-Methoden
    void SetRefObject( SwServerObject* pObj );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }
    BOOL IsServer() const                       {  return refObj.Is(); }

    // --> OD 2007-10-10 #i81002#
    // made virtual and thus no longer inline
    // to access start and end of a bookmark.
    // start and end may be the same
    virtual const SwPosition* BookmarkStart() const;
    virtual const SwPosition* BookmarkEnd() const;
    // <--

    // --> OD 2007-09-26 #i81002#
    virtual void SetBookmarkPos( const SwPosition* pNewPos1 );
    virtual void SetOtherBookmarkPos( const SwPosition* pNewPos2 );
    // <--

private:
    // fuer METWARE:
    // es wird (vorerst) nicht kopiert und nicht zugewiesen
    SwBookmark(const SwBookmark &);
    SwBookmark &operator=(const SwBookmark &);
};

class SwMark: public SwBookmark
{
public:
    SwMark( const SwPosition& aPos,
            const KeyCode& rCode,
            const String& rName, const String& rShortName);
};

class SwUNOMark: public SwBookmark
{
public:
    // --> OD 2007-09-26 #i81002#
    SwUNOMark( const SwPaM& aPaM,
               const KeyCode& rCode,
               const String& rName, const String& rShortName);
    // <--
};


#endif
