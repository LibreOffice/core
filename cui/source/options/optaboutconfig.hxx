/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _SVX_OPTABOUTCONFIG_HXX
#define _SVX_OPTABOUTCONFIG_HXX

#include <svx/simpltabl.hxx>

namespace svx
{
    class OptHeaderTabListBox;
}


class SvxAboutConfigTabPage : public SfxTabPage
{
private:
    SvxSimpleTableContainer* m_pPrefCtrl;
    //In case of tab page we can think a revert button
    //PushButton* m_pDefaultBtn
    PushButton* m_pEditBtn;

    ::svx::OptHeaderTabListBox* pPrefBox;
public:
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
