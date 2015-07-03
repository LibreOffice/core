#include <svx/fontwork.hxx>
#include <svx/svdview.hxx>
#include <svx/svdotext.hxx>
#include <svl/whiter.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <svl/itemiter.hxx>
#include <svl/srchitem.hxx>
#include <svx/tbxcustomshapes.hxx>
#include <uitool.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <docstat.hxx>
#include <IDocumentStatistics.hxx>
#include <tools/diagnose_ex.h>
#include "drlinesh.hxx"
#include <svx/svdoashp.hxx>
#include <svx/xtable.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/shell.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/xbtmpit.hxx>
#include <sfx2/dispatch.hxx>
#include "swundo.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "drwbassh.hxx"
#include "drawsh.hxx"

#define SwDrawLineShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include "swabstdlg.hxx"
#include "misc.hrc"
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SFX_IMPL_INTERFACE(SwDrawLineShell, SwDrawBaseShell)

void SwDrawLineShell::InitInterface_Impl()
{

    GetStaticInterface()->RegisterPopupMenu(SW_RES(MN_DRAWLINE_POPUPMENU));

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_DRAWLINE_TOOLBOX);

    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());

}

TYPEINIT1(SwDrawLineShell,SwDrawBaseShell)

SwDrawLineShell::SwDrawLineShell(SwView &_rView) :
    SwDrawBaseShell(_rView)
{
    SetHelpId(SW_DRAWLINESHELL);
    SetName(OUString("DrawLine"));

    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_DrawLine));
}

void SwDrawLineShell::ExecDrawDlg(SfxRequest& rReq)
{
    SwWrtShell*     pSh     = &GetShell();
    SdrView*        pView   = pSh->GetDrawView();
    SdrModel*       pDoc    = pView->GetModel();
    bool            bChanged = pDoc->IsChanged();
    pDoc->SetChanged(false);

    SfxItemSet aNewAttr( pDoc->GetItemPool() );
    pView->GetAttributes( aNewAttr );

    GetView().NoRotate();

    switch (rReq.GetSlot())
    {

        case SID_ATTRIBUTES_LINE:
        {
            bool bHasMarked = pView->AreObjectsMarked();

            const SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
                pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialog creation failed!");
            boost::scoped_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxLineTabDialog( NULL,
                    &aNewAttr,
                pDoc,
                pObj,
                bHasMarked));
            OSL_ENSURE(pDlg, "Dialog creation failed!");
            if (pDlg->Execute() == RET_OK)
            {
                pSh->StartAction();
                if(bHasMarked)
                    pView->SetAttrToMarked(*pDlg->GetOutputItemSet(), false);
                else
                    pView->SetDefaultAttr(*pDlg->GetOutputItemSet(), false);
                pSh->EndAction();

                static sal_uInt16 aInval[] =
                {
                    SID_ATTR_LINE_STYLE,                // ( SID_SVX_START + 169 )
                    SID_ATTR_LINE_DASH,                 // ( SID_SVX_START + 170 )
                    SID_ATTR_LINE_WIDTH,                // ( SID_SVX_START + 171 )
                    SID_ATTR_LINE_COLOR,                // ( SID_SVX_START + 172 )
                    SID_ATTR_LINE_START,                // ( SID_SVX_START + 173 )
                    SID_ATTR_LINE_END,                  // ( SID_SVX_START + 174 )
                    SID_ATTR_LINE_TRANSPARENCE,         // (SID_SVX_START+1107)
                    SID_ATTR_LINE_JOINT,                // (SID_SVX_START+1110)
                    SID_ATTR_LINE_CAP,                  // (SID_SVX_START+1111)
                    0
                };

                GetView().GetViewFrame()->GetBindings().Invalidate(aInval);
            }
        }
        break;

        default:
            break;
    }

    if (pDoc->IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pDoc->SetChanged(true);
}

void SwDrawLineShell::ExecDrawAttrArgs(SfxRequest& rReq)
{
    SwWrtShell* pSh   = &GetShell();
    SdrView*    pView = pSh->GetDrawView();
    const SfxItemSet* pArgs = rReq.GetArgs();
    bool        bChanged = pView->GetModel()->IsChanged();
    pView->GetModel()->SetChanged(false);

    GetView().NoRotate();

    if (pArgs)
    {
        if(pView->AreObjectsMarked())
            pView->SetAttrToMarked(*rReq.GetArgs(), false);
        else
            pView->SetDefaultAttr(*rReq.GetArgs(), false);
    }
    else
    {
        SfxDispatcher* pDis = pSh->GetView().GetViewFrame()->GetDispatcher();
        switch (rReq.GetSlot())
        {
            case SID_ATTR_LINE_STYLE:
            case SID_ATTR_LINE_DASH:
            case SID_ATTR_LINE_WIDTH:
            case SID_ATTR_LINE_COLOR:
            case SID_ATTR_LINE_TRANSPARENCE:
            case SID_ATTR_LINE_JOINT:
            case SID_ATTR_LINE_CAP:
                pDis->Execute(SID_ATTRIBUTES_LINE);
                break;
        }
    }
    if (pView->GetModel()->IsChanged())
        GetShell().SetModified();
    else
        if (bChanged)
            pView->GetModel()->SetChanged(true);
}

void SwDrawLineShell::GetDrawAttrState(SfxItemSet& rSet)
{
    SdrView* pSdrView = GetShell().GetDrawView();

    if (pSdrView->AreObjectsMarked())
    {
        bool bDisable = Disable( rSet );

        if( !bDisable )
            pSdrView->GetAttributes( rSet );
    }
    else
        rSet.Put(pSdrView->GetDefaultAttr());
}

