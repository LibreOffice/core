/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleText.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:19:19 $
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
#include <svx/svxenum.hxx>
#endif

#include <memory>

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
    virtual             ~ScAccessibleTextData() {}

    virtual ScAccessibleTextData* Clone() const = 0;

    virtual void        Notify( SfxBroadcaster& /* rBC */, const SfxHint& /* rHint */ ) {}

    virtual SvxTextForwarder* GetTextForwarder() = 0;
    virtual SvxViewForwarder* GetViewForwarder() = 0;
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate ) = 0;
    virtual SfxBroadcaster& GetBroadcaster() const { return maBroadcaster; }

    virtual void                UpdateData() = 0;
    virtual void                SetDoUpdate(sal_Bool bValue) = 0;
    virtual sal_Bool            IsDirty() const = 0;

private:
    mutable SfxBroadcaster maBroadcaster;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }
};


// ============================================================================

class ScAccessibleCellBaseTextData : public ScAccessibleTextData,
                                     public ScCellTextData
{
public:
                        ScAccessibleCellBaseTextData(ScDocShell* pDocShellP,
                            const ScAddress& rP)
                            : ScCellTextData(pDocShellP, rP) {}
    virtual             ~ScAccessibleCellBaseTextData() {}
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) { ScCellTextData::Notify(rBC, rHint); }

    virtual void                UpdateData() { ScCellTextData::UpdateData(); }
    virtual void                SetDoUpdate(sal_Bool bValue) { ScCellTextData::SetDoUpdate(bValue); }
    virtual sal_Bool            IsDirty() const { return ScCellTextData::IsDirty(); }
};


// ============================================================================

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

class ScAccessibleCellTextData : public ScAccessibleCellBaseTextData
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
protected:
    virtual void            GetCellText(const ScAddress& rCellPos, String& rText);
private:
    ScViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    ScTabViewShell* mpViewShell;
    ScSplitPos meSplitPos;
    sal_Bool mbViewEditEngine;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
    ScDocShell* GetDocShell(ScTabViewShell* pViewShell);
};


// ============================================================================

class ScAccessibleEditObjectTextData : public ScAccessibleTextData
{
public:
                        ScAccessibleEditObjectTextData(EditView* pEditView, Window* pWin);
    virtual             ~ScAccessibleEditObjectTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate );

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return sal_False; }

    DECL_LINK( NotifyHdl, EENotify* );
protected:
    ScEditObjectViewForwarder* mpViewForwarder;
    ScEditViewForwarder* mpEditViewForwarder;
    EditView* mpEditView;
    EditEngine* mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
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
    virtual             ~ScAccessiblePreviewCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
    ScDocShell* GetDocShell(ScPreviewShell* pViewShell);
};


// ============================================================================

class ScAccessiblePreviewHeaderCellTextData : public ScAccessibleCellBaseTextData
{
public:
                        ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
                            const String& rText, const ScAddress& rP, sal_Bool bColHeader, sal_Bool bRowHeader);
    virtual             ~ScAccessiblePreviewHeaderCellTextData();

    virtual ScAccessibleTextData* Clone() const;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    DECL_LINK( NotifyHdl, EENotify* );
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell* mpViewShell;
    String          maText;
    sal_Bool        mbColHeader;
    sal_Bool        mbRowHeader;

    // prevent the using of this method of the base class
    ScSharedCellEditSource* GetOriginalSource() { return NULL; }

    using ScAccessibleCellBaseTextData::GetDocShell;
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

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return sal_False; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
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

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SvxTextForwarder* GetTextForwarder();
    virtual SvxViewForwarder* GetViewForwarder();
    virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool /* bCreate */ ) { return NULL; }

    virtual void                UpdateData() {  }
    virtual void                SetDoUpdate(sal_Bool /* bValue */) {  }
    virtual sal_Bool            IsDirty() const { return sal_False; }
private:
    ScPreviewViewForwarder* mpViewForwarder;
    ScPreviewShell*         mpViewShell;
    ScEditEngineDefaulter*  mpEditEngine;
    SvxEditEngineForwarder* mpForwarder;
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
    virtual void                SetDoUpdate( sal_Bool /* bValue */ ) {}
    virtual sal_Bool            IsDirty() const { return sal_False; }
};


// ============================================================================

#endif
