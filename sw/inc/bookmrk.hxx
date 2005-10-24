/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookmrk.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-24 15:29:12 $
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

#ifndef _BKMRKE_HXX
#include <bkmrke.hxx>
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
    friend class Sw3IoImp;      // fuers Setzen der Position(en)

    SwPosition *pPos1, *pPos2;  // wird im CTOR gesetzt, im DTOR geloescht
                                // pPos1 is always != 0, pPos2 may be 0
    SwServerObjectRef refObj;   // falls DataServer -> Pointer gesetzt

protected:
    SvxMacro    aStartMacro;
    SvxMacro    aEndMacro;
    String      aName;
    String      aShortName;
    KeyCode     aCode;
    BOOKMARK_TYPE eMarkType;

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
    void SetName(const String& rNewName) { aName = rNewName; }
    const String& GetName() const { return aName; }
    // nicht undofaehig
    void SetShortName(const String& rNewSName) { aShortName = rNewSName; }
    const String& GetShortName() const { return aShortName; }
    // nicht undofaehig
    void SetKeyCode(const KeyCode& rNewCode) { aCode = rNewCode; }
    const KeyCode& GetKeyCode() const { return aCode; }

    // Vergleiche auf Basis der Dokumentposition
    BOOL operator < (const SwBookmark &) const;
    BOOL operator ==(const SwBookmark &) const;
    // falls man wirklich auf gleiche Position abfragen will.
    BOOL IsEqualPos( const SwBookmark &rBM ) const;

    BOOL IsBookMark() const     { return BOOKMARK == eMarkType; }
    BOOL IsMark() const         { return MARK == eMarkType; }
    BOOL IsDDEMark() const      { return DDE_BOOKMARK == eMarkType; }
    BOOL IsUNOMark() const      { return UNO_BOOKMARK == eMarkType; }
    BOOL IsHiddenBookMark() const   { return BOOKMARK_HIDDEN == eMarkType; }
    void SetType( BOOKMARK_TYPE eNewType )  { eMarkType = eNewType; }
    BOOKMARK_TYPE GetType() const   { return eMarkType; }

    void SetStartMacro(const SvxMacro& rSt)     { aStartMacro = rSt; }
    const SvxMacro& GetStartMacro()             { return aStartMacro; }
    void SetEndMacro(const SvxMacro& rSt)       { aEndMacro = rSt; }
    const SvxMacro& GetEndMacro()               { return aEndMacro; }

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
