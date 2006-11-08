/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookmrk.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 13:20:14 $
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
#ifndef _BOOKMRK_HXX
#define _BOOKMRK_HXX

#include "hintids.hxx"      //die Ids der Attribute, vor macitem damit die
                            //die Attribut richtig angezogen werden.
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef IDOCUMENTBOOKMARKACCESS_HXX_INCLUDED
#include <IDocumentBookmarkAccess.hxx>
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif


struct SwPosition;  // fwd Decl. wg. UI

class SwBookmark : public SwModify
{
    friend class SwDoc;         // fuers Loeschen
//  friend class Sw3IoImp;      // fuers Setzen der Position(en)

    SwPosition *pPos1, *pPos2;  // wird im CTOR gesetzt, im DTOR geloescht
                                // pPos1 is always != 0, pPos2 may be 0
    SwServerObjectRef refObj;   // falls DataServer -> Pointer gesetzt

protected:
    SvxMacro    aStartMacro;
    SvxMacro    aEndMacro;
    String      aName;
    String      aShortName;
    KeyCode     aCode;
    IDocumentBookmarkAccess::BookmarkType eMarkType;

public:
    TYPEINFO();

    SwBookmark(const SwPosition& aPos);
    SwBookmark(const SwPosition& aPos,
        const KeyCode& rCode,
        const String& rName, const String& rShortName);
    // Beim Loeschen von Text werden Bookmarks mitgeloescht!
    virtual ~SwBookmark();

    const SwPosition& GetPos() const { return *pPos1; }
    const SwPosition* GetOtherPos() const { return pPos2; }

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

    BOOL IsBookMark() const     { return IDocumentBookmarkAccess::BOOKMARK == eMarkType; }
    BOOL IsMark() const         { return IDocumentBookmarkAccess::MARK == eMarkType; }
    BOOL IsDDEMark() const      { return IDocumentBookmarkAccess::DDE_BOOKMARK == eMarkType; }
    BOOL IsUNOMark() const      { return IDocumentBookmarkAccess::UNO_BOOKMARK == eMarkType; }
    BOOL IsHiddenBookMark() const { return IDocumentBookmarkAccess::HIDDEN_BOOKMARK == eMarkType; }
    void SetType( IDocumentBookmarkAccess::BookmarkType eNewType )  { eMarkType = eNewType; }
    IDocumentBookmarkAccess::BookmarkType GetType() const   { return eMarkType; }

    void SetStartMacro(const SvxMacro& rSt)     { aStartMacro = rSt; }
    const SvxMacro& GetStartMacro()             { return aStartMacro; }
    void SetEndMacro(const SvxMacro& rSt)       { aEndMacro = rSt; }

        // Daten Server-Methoden
    void SetRefObject( SwServerObject* pObj );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }
    BOOL IsServer() const                       {  return refObj.Is(); }

    // to access start and end of a bookmark.
    // start and end may be the same
    const SwPosition *Start() const
    {
        return pPos2 ? (*pPos1 <= *pPos2 ? pPos1 : pPos2) : pPos1;
    }
    const SwPosition *End()   const
    {
        return pPos2 ? (*pPos1 >= *pPos2 ? pPos1 : pPos2) : pPos1;
    }

private:
    // fuer METWARE:
    // es wird (vorerst) nicht kopiert und nicht zugewiesen
    SwBookmark(const SwBookmark &);
    SwBookmark &operator=(const SwBookmark &);
};

class SwMark: public SwBookmark
{
public:
    SwMark(const SwPosition& aPos,
        const KeyCode& rCode,
        const String& rName, const String& rShortName);
};

class SwUNOMark: public SwBookmark
{
public:
    SwUNOMark(const SwPosition& aPos,
        const KeyCode& rCode,
        const String& rName, const String& rShortName);
};


#endif
