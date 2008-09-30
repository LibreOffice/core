/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.hxx,v $
 * $Revision: 1.7.32.1 $
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

// ============================================================================

#ifndef SC_POSTIT_HXX
#define SC_POSTIT_HXX

#include <boost/shared_ptr.hpp>

#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/syslocale.hxx>
#include <tools/gen.hxx>
#include <svx/editobj.hxx>
#include <svx/svddef.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include "global.hxx"
#include <com/sun/star/drawing/XShape.hpp>

//  indicative text length for a note object
#define SC_NOTE_SMALLTEXT   100

class EditTextObject;
class ScDocument;

//==================================================================
// Notes
//==================================================================

class ScPostIt
{
private:
    typedef ::boost::shared_ptr< EditTextObject > EditObjPtr;
    EditObjPtr         mpEditObj;
    ScDocument*        mpDoc;
    String             maStrDate;
    String             maStrAuthor;
    BOOL               mbShown;
    Rectangle          maRectangle;
    SfxItemSet         maItemSet;

public:
    explicit ScPostIt( ScDocument* pDoc );
    ScPostIt( const String& rText, ScDocument* pDoc );
    ScPostIt( const EditTextObject* pTextObj, ScDocument* pDoc );
    ScPostIt( const ScPostIt& rNote, ScDocument* pDoc );

    ~ScPostIt();

    inline const EditTextObject* GetEditTextObject() const { return mpEditObj.get();     }

        String          GetText()   const;
    const String&       GetDate()   const { return maStrDate;   }
    const String&       GetAuthor() const { return maStrAuthor; }
    BOOL            IsShown()   const { return mbShown;     }

    void                SetEditTextObject( const EditTextObject* pTextObj);
    void            SetText( const String& rText );
    void            SetDate( const String& rNew )   { maStrDate   = rNew; }
    void            SetAuthor( const String& rNew ) { maStrAuthor = rNew; }
    void            SetShown( BOOL bNew )   { mbShown    = bNew; }

    void                    Clear() { mpEditObj.reset(); maStrDate.Erase(); maStrAuthor.Erase(); mbShown = FALSE; }
    void            AutoStamp();

    BOOL            IsEmpty() const;
    Rectangle   DefaultRectangle(const ScAddress& rPos) const ;
    Rectangle   MimicOldRectangle(const ScAddress& rPos) const ;
    SfxItemSet  DefaultItemSet() const ;
    inline const Rectangle& GetRectangle() const {return maRectangle;}
    void         SetRectangle(const Rectangle& aRect);
    inline const SfxItemSet&    GetItemSet() const {return maItemSet;}
    void         SetItemSet(const SfxItemSet& aItemSet);
    void         SetAndApplyItemSet(const SfxItemSet& aItemSet);

    void         InsertObject( SdrCaptionObj* pObj, ScDocument& rDoc, SCTAB nTab, sal_Bool bVisible) const;
    void         RemoveObject( SdrCaptionObj* pObj, ScDocument& rDoc, SCTAB nTab) const;

    const ScPostIt& operator= ( const ScPostIt& rCpy );
    inline int      operator==( const ScPostIt& rPostIt ) const;
    int         operator!=( const ScPostIt& rPostIt ) const { return !(operator==(rPostIt)); }

    friend inline SvStream&     operator>>( SvStream& rStream, ScPostIt& rPostIt );
    friend inline SvStream&     operator<<( SvStream& rStream, const ScPostIt& rPostIt );
};


inline int ScPostIt::operator==( const ScPostIt& rPostIt ) const
{
    return (  ScGlobal::EETextObjEqual (mpEditObj.get(), rPostIt.mpEditObj.get())
        &&  maStrDate   == rPostIt.maStrDate
        &&  maStrAuthor == rPostIt.maStrAuthor
        &&  mbShown == rPostIt.mbShown
        &&  maRectangle == rPostIt.maRectangle
        &&  maItemSet   == rPostIt.maItemSet );
}

// ============================================================================

#endif
