/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant - Child Window for AgenticPanel
 */

#ifndef INCLUDED_OFFICELABS_AGENTICPANELCHILDWINDOW_HXX
#define INCLUDED_OFFICELABS_AGENTICPANELCHILDWINDOW_HXX

#include <officelabs/officelabsdllapi.h>
#include <sfx2/childwin.hxx>

namespace officelabs {

class OFFICELABS_DLLPUBLIC AgenticPanelChildWindow final : public SfxChildWindow
{
public:
    AgenticPanelChildWindow(vcl::Window* pParent, sal_uInt16 nId,
                            SfxBindings* pBindings, SfxChildWinInfo* pInfo);
    virtual ~AgenticPanelChildWindow() override;

    SFX_DECL_CHILDWINDOW_WITHID(AgenticPanelChildWindow);
};

} // namespace officelabs

#endif // INCLUDED_OFFICELABS_AGENTICPANELCHILDWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
