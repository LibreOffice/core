/*************************************************************************
 *
 *  $RCSfile: bookmrk.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:24 $
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
#ifndef _SO2REF_HXX //autogen
#include <so3/so2ref.hxx>
#endif
#ifndef _BKMRKE_HXX //autogen
#include <bkmrke.hxx>
#endif

#include "calbck.hxx"

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
class SvPseudoObject;
SO2_DECL_REF( SwServerObject )
#endif


struct SwPosition;  // fwd Decl. wg. UI

class SwBookmark : public SwModify
{
    friend class SwDoc;         // fuers Loeschen
    friend class Sw3IoImp;      // fuers Setzen der Position(en)

    SwPosition *pPos1, *pPos2;  // wird im CTOR gesetzt, im DTOR geloescht
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
    ~SwBookmark();

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
    void SetType( BOOKMARK_TYPE eNewType )  { eMarkType = eNewType; }

    void SetStartMacro(const SvxMacro& rSt)     { aStartMacro = rSt; }
    const SvxMacro& GetStartMacro()             { return aStartMacro; }
    void SetEndMacro(const SvxMacro& rSt)       { aEndMacro = rSt; }
    const SvxMacro& GetEndMacro()               { return aEndMacro; }

        // Daten Server-Methoden
    void SetRefObject( SvPseudoObject* pObj );
    const SwServerObject* GetObject() const     {  return &refObj; }
          SwServerObject* GetObject()           {  return &refObj; }
    BOOL IsServer() const                       {  return refObj.Is(); }

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
