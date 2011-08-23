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

#ifndef _SVX_SVXENUM_HXX
#include <bf_svx/svxenum.hxx>
#endif

#include <memory>
namespace binfilter {

class ScCellTextData;
class ScDocShell;
class ScViewForwarder;
class ScEditObjectViewForwarder;
class ScPreviewViewForwarder;
class ScEditViewForwarder;
class ScPreviewShell;
class EditTextObject;
class ScCsvViewForwarder;


// ============================================================================

class ScAccessibleTextData : public SfxListener
{
public:
                        ScAccessibleTextData() {}
    virtual				~ScAccessibleTextData() {}

    virtual	ScAccessibleTextData* Clone() const = 0;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) {}

    virtual SvxTextForwarder* GetTextForwarder() = 0;
    virtual SvxViewForwarder* GetViewForwarder() = 0;
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) = 0;
    virtual SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

    virtual void				UpdateData() = 0;
    virtual void				SetDoUpdate(sal_Bool bValue) = 0;
    virtual sal_Bool			IsDirty() const = 0;

private:
    mutable SfxBroadcaster maBroadcaster;

    // prevent the using of this method of the base class
    ScSharedCellEditSource*	GetOriginalSource() { return NULL; }
};


// ============================================================================

class ScAccessibleCellBaseTextData : public ScAccessibleTextData,
                                     public ScCellTextData
{
public:
                        ScAccessibleCellBaseTextData(ScDocShell* pDocShell,
                            const ScAddress& rP)
                            : ScCellTextData(pDocShell, rP) {}
    virtual             ~ScAccessibleCellBaseTextData() {}
    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) { ScCellTextData::Notify(rBC, rHint); }

    virtual void				UpdateData() { ScCellTextData::UpdateData(); }
    virtual void				SetDoUpdate(sal_Bool bValue) { ScCellTextData::SetDoUpdate(bValue); }
    virtual sal_Bool			IsDirty() const { return ScCellTextData::IsDirty(); }
};


// ============================================================================

//	ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessibleCellTextData();
//						ScTabViewShell* pViewShell,
//							const ScAddress& rP, ScSplitPos eSplitPos);
    virtual				~ScAccessibleCellTextData();

    virtual	ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    DECL_LINK( NotifyHdl, EENotify* );
protected:
    virtual void            GetCellText(const ScAddress& rCellPos, String& rText);    
private:
    ScViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
//	ScTabViewShell*	mpViewShell;
    ScSplitPos meSplitPos;
    sal_Bool mbViewEditEngine;

    // prevent the using of this method of the base class
    ScSharedCellEditSource*	GetOriginalSource() { return NULL; }

//	ScDocShell* GetDocShell(ScTabViewShell* pViewShell);
};


// ============================================================================

class ScAccessibleEditObjectTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleEditObjectTextData(EditView* pEditView, Window* pWin);
    virtual				~ScAccessibleEditObjectTextData();

    virtual	ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    virtual void				UpdateData() {  }
    virtual void				SetDoUpdate(sal_Bool bValue) {  }
    virtual sal_Bool			IsDirty() const { return sal_False; }

    DECL_LINK( NotifyHdl, EENotify* );
protected:
    ScEditObjectViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    EditView* mpEditView;
    EditEngine* mpEditEngine;
    SvxEditEngineForwarder*	mpForwarder;
    Window* mpWindow;
};


// ============================================================================

class ScAccessibleEditLineTextData : public ScAccessibleEditObjectTextData
{
public:
                        ScAccessibleEditLineTextData(EditView* pEditView, Window* pWin);
    virtual             ~ScAccessibleEditLineTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    void Dispose();
    void TextChanged();
    void StartEdit();
    void EndEdit();
private:
    void ResetEditMode();

    sal_Bool mbEditEngineCreated;
};


// ============================================================================

class ScAccessiblePreviewCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP);
    virtual				~ScAccessiblePreviewCellTextData();

    virtual	ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*	mpViewShell;

    // prevent the using of this method of the base class
    ScSharedCellEditSource*	GetOriginalSource() { return NULL; }

    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};


// ============================================================================

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const String& rText, const ScAddress& rP, sal_Bool bColHeader, sal_Bool bRowHeader);
    virtual				~ScAccessiblePreviewHeaderCellTextData();

    virtual	ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*	mpViewShell;
    String			maText;
    sal_Bool		mbColHeader;
    sal_Bool		mbRowHeader;

    // prevent the using of this method of the base class
    ScSharedCellEditSource*	GetOriginalSource() { return NULL; }

    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};


// ============================================================================

class ScAccessibleHeaderTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, sal_Bool bHeader, SvxAdjust eAdjust);
    virtual             ~ScAccessibleHeaderTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    virtual void				UpdateData() {  }
    virtual void				SetDoUpdate(sal_Bool bValue) {  }
    virtual sal_Bool			IsDirty() const { return sal_False; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*	mpEditEngine;
    SvxEditEngineForwarder*	mpForwarder;
    ScDocShell*             mpDocSh;
    const EditTextObject*   mpEditObj;
    sal_Bool                mbHeader;
    sal_Bool                mbDataValid;
    SvxAdjust               meAdjust;
};


// ============================================================================

class ScAccessibleNoteTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const String& sText, const ScAddress& aCellPos, sal_Bool bMarkNote);
    virtual             ~ScAccessibleNoteTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void		Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) { return NULL; }

    virtual void				UpdateData() {  }
    virtual void				SetDoUpdate(sal_Bool bValue) {  }
    virtual sal_Bool			IsDirty() const { return sal_False; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*	mpEditEngine;
    SvxEditEngineForwarder*	mpForwarder;
    ScDocShell*             mpDocSh;
    String                  msText;
    ScAddress               maCellPos;
    sal_Bool                mbMarkNote;
    sal_Bool                mbDataValid;
};


// ============================================================================

class ScAccessibleCsvTextData : public ScAccessibleTextData
{
private:
    typedef ::std::auto_ptr< SvxTextForwarder > TextForwarderPtr;
    typedef ::std::auto_ptr< ScCsvViewForwarder > ViewForwarderPtr;

    Window*                     mpWindow;
    EditEngine*                 mpEditEngine;
    TextForwarderPtr            mpTextForwarder;
    ViewForwarderPtr            mpViewForwarder;
    String                      maCellText;
    Rectangle                   maBoundBox;
    Size                        maCellSize;

public:
    explicit                    ScAccessibleCsvTextData(
                                    Window* pWindow,
                                    EditEngine* pEditEngine,
                                    const String& rCellText,
                                    const Rectangle& rBoundBox,
                                    const Size& rCellSize );
    virtual                     ~ScAccessibleCsvTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*   GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    virtual void                UpdateData() {}
    virtual void                SetDoUpdate( sal_Bool bValue ) {}
    virtual sal_Bool            IsDirty() const { return sal_False; }
};


// ============================================================================

} //namespace binfilter
#endif
