/*************************************************************************
 *
 *  $RCSfile: postit.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-13 17:21:26 $
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

// ============================================================================

#ifndef SC_POSTIT_HXX
#define SC_POSTIT_HXX

#include <boost/shared_ptr.hpp>

#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _SVDDEF_HXX
#include <svx/svddef.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDCAPT_HXX
#include <svx/svdocapt.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

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

inline SvStream& operator>>( SvStream& rStream, ScPostIt& rPostIt )
{
    //  without bShown !!!

    CharSet eSet = rStream.GetStreamCharSet();
    String aText = rPostIt.GetText();
    rStream.ReadByteString( aText, eSet );
    rStream.ReadByteString( rPostIt.maStrDate, eSet );
    rStream.ReadByteString( rPostIt.maStrAuthor, eSet );

    return rStream;
}

inline SvStream& operator<<( SvStream& rStream, const ScPostIt& rPostIt )
{
    //  without bShown !!!

    CharSet eSet = rStream.GetStreamCharSet();
    String aText = rPostIt.GetText();
    rStream.WriteByteString( aText, eSet );
    rStream.WriteByteString( rPostIt.maStrDate, eSet );
    rStream.WriteByteString( rPostIt.maStrAuthor, eSet );

    return rStream;
}

// ============================================================================

#endif
