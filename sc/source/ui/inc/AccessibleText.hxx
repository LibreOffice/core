/*************************************************************************
 *
 *  $RCSfile: AccessibleText.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2002-03-21 07:15:04 $
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

#ifndef _SC_ACCESSIBLETEXT_HXX
#define _SC_ACCESSIBLETEXT_HXX

#ifndef SC_TEXTSUNO_HXX
#include "textuno.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_VIEWDATA_HXX
#include "viewdata.hxx"
#endif

class ScCellTextData;
class ScDocShell;
class ScViewForwarder;
class ScEditViewForwarder;
class ScPreviewShell;

class ScAccessibleTextData : public ScCellTextData
{
public:
                        ScAccessibleTextData(ScDocShell* pDocShell,
                            const ScAddress& rP)
                            : ScCellTextData(pDocShell, rP) {}
    virtual             ~ScAccessibleTextData() {}

    virtual ScAccessibleTextData* Clone() const = NULL;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) {}

    virtual SvxTextForwarder* GetTextForwarder() = NULL;
    virtual SvxViewForwarder* GetViewForwarder() = NULL;
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) = NULL;
    virtual SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

private:
    mutable SfxBroadcaster maBroadcaster;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }
};

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos);
    virtual             ~ScAccessibleCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    ScTabViewShell* mpViewShell;
    ScSplitPos meSplitPos;
    sal_Bool mbViewEditEngine;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    ScDocShell* GetDocShell(ScTabViewShell* pViewShell);
};

class ScAccessiblePreviewCellTextData : public ScAccessibleTextData
{
public:
                        ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP);
    virtual             ~ScAccessiblePreviewCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const String& rText);
    virtual             ~ScAccessiblePreviewHeaderCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;
    String          maText;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};

#endif
