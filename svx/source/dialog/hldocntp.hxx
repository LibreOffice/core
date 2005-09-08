/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hldocntp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:13:55 $
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
#ifndef _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX
#define _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - New Document
|*
\************************************************************************/

class SvxHyperlinkNewDocTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpNewDoc;
    RadioButton         maRbtEditNow;
    RadioButton         maRbtEditLater;
    FixedText           maFtPath;
    SvxHyperURLBox      maCbbPath;
    ImageButton         maBtCreate;
    FixedText           maFtDocTypes;
    ListBox             maLbDocTypes;

    sal_Bool            ImplGetURLObject( const String& rPath, const String& rBase, INetURLObject& aURLObject ) const;
    void                FillDocumentList ();

    DECL_LINK (ClickNewHdl_Impl          , void * );        // Button : New

    Image GetImage( USHORT nId );
    void ReadURLFile( const String& rFile, String& rTitle, String& rURL, sal_Int32& rIconId, BOOL* pShowAsFolder);//, String* pFrame, String* pOpenAs, String* pDefTempl, String* pDefURL );
    //String ReadURL_Impl( Config& rURLFile, const DirEntry& rFile );

protected:
    void FillDlgFields     ( String& aStrURL );
    void GetCurentItemData ( String& aStrURL, String& aStrName,
                             String& aStrIntName, String& aStrFrame,
                             SvxLinkInsertMode& eMode );

public:
    SvxHyperlinkNewDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkNewDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual BOOL        AskApply ();
    virtual void        DoApply ();

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_DOCNEW_HYPERLINK_HXX
