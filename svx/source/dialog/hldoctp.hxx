/*************************************************************************
 *
 *  $RCSfile: hldoctp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 18:29:00 $
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
#ifndef _SVX_TABPAGE_DOC_HYPERLINK_HXX
#define _SVX_TABPAGE_DOC_HYPERLINK_HXX

#include "hltpbase.hxx"

/*************************************************************************
|*
|* Tabpage : Hyperlink - Document
|*
\************************************************************************/

class SvxHyperlinkDocTp : public SvxHyperlinkTabPageBase
{
private:
    FixedLine           maGrpDocument;
    FixedText           maFtPath;
    SvxHyperURLBox      maCbbPath;
    ImageButton         maBtFileopen;

    FixedLine           maGrpTarget;
    FixedText           maFtTarget;
    Edit                maEdTarget;
    FixedText           maFtURL;
    FixedText           maFtFullURL;
    ImageButton         maBtBrowse;

    String              maStrURL;

    BOOL                mbMarkWndOpen;

    DECL_LINK (ClickFileopenHdl_Impl  , void * );       // Button : Fileopen
    DECL_LINK (ClickTargetHdl_Impl    , void * );       // Button : Target

    DECL_LINK (ModifiedPathHdl_Impl  , void * );        // Contens of combobox "Path" modified
    DECL_LINK (ModifiedTargetHdl_Impl, void * );        // Contens of editfield "Target" modified

    DECL_LINK (LostFocusPathHdl_Impl,  void * );        // Combobox "path" lost its focus

    DECL_LINK (TimeoutHdl_Impl      ,  Timer * );       // Handler for timer -timeout

    enum EPathType { Type_Unknown, Type_Invalid,
                     Type_ExistsFile, Type_File,
                     Type_ExistsDir, Type_Dir };
    EPathType GetPathType ( String& aStrPath );

protected:
    void FillDlgFields     ( String& aStrURL );
    void GetCurentItemData ( String& aStrURL, String& aStrName,
                             String& aStrIntName, String& aStrFrame,
                             SvxLinkInsertMode& eMode );
    virtual BOOL   ShouldOpenMarkWnd () {return mbMarkWndOpen;}
    virtual void   SetMarkWndShouldOpen (BOOL bOpen) {mbMarkWndOpen=bOpen;}
    String GetCurrentURL    ();

public:
    SvxHyperlinkDocTp ( Window *pParent, const SfxItemSet& rItemSet);
    ~SvxHyperlinkDocTp ();

    static  IconChoicePage* Create( Window* pWindow, const SfxItemSet& rItemSet );

    virtual void        SetMarkStr ( String& aStrMark );

    virtual void        SetInitFocus();
};


#endif // _SVX_TABPAGE_DOC_HYPERLINK_HXX
