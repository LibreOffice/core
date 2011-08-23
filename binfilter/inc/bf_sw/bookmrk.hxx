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
#ifndef _BOOKMRK_HXX
#define _BOOKMRK_HXX

                            //die Attribut richtig angezogen werden.
#ifndef _SFXMACITEM_HXX
#include <bf_svtools/macitem.hxx>
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
namespace binfilter {

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif


struct SwPosition;	// fwd Decl. wg. UI

class SwBookmark : public SwModify
{
    friend class SwDoc;			// fuers Loeschen
    friend class Sw3IoImp;		// fuers Setzen der Position(en)

    SwPosition *pPos1, *pPos2;	// wird im CTOR gesetzt, im DTOR geloescht
      SwServerObjectRef refObj;	// falls DataServer -> Pointer gesetzt

protected:
    SvxMacro 	aStartMacro;
    SvxMacro 	aEndMacro;
    String		aName;
    String		aShortName;
    KeyCode		aCode;
    BOOKMARK_TYPE eMarkType;

public:

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

    BOOL IsBookMark() const 	{ return BOOKMARK == eMarkType; }
    BOOL IsMark() const 		{ return MARK == eMarkType; }
    BOOL IsDDEMark() const 		{ return DDE_BOOKMARK == eMarkType; }
    BOOL IsUNOMark() const 		{ return UNO_BOOKMARK == eMarkType; }
    void SetType( BOOKMARK_TYPE eNewType ) 	{ eMarkType = eNewType; }
    BOOKMARK_TYPE GetType() const 	{ return eMarkType; }

    void SetStartMacro(const SvxMacro& rSt)		{ aStartMacro = rSt; }
    const SvxMacro& GetStartMacro()				{ return aStartMacro; }
    void SetEndMacro(const SvxMacro& rSt) 		{ aEndMacro = rSt; }
    const SvxMacro& GetEndMacro() 				{ return aEndMacro; }

        // Daten Server-Methoden
    const SwServerObject* GetObject() const		{  return &refObj; }
          SwServerObject* GetObject() 			{  return &refObj; }

private:
    // fuer METWARE:
    // es wird (vorerst) nicht kopiert und nicht zugewiesen
    SwBookmark(const SwBookmark &);
    SwBookmark &operator=(const SwBookmark &);
};


class SwUNOMark: public SwBookmark
{
public:
    SwUNOMark(const SwPosition& aPos,
        const KeyCode& rCode,
        const String& rName, const String& rShortName);
};


} //namespace binfilter
#endif
