/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX

#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/text/XTextDocument.hpp>

class SfxBindings;

namespace sw::sidebar {

class AIAssistantPanel : public PanelLayout
{
private:
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::TextView> m_xChatHistory;
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xSendButton;
    
    SfxBindings* m_pBindings;
    
    DECL_LINK(SendClickHdl, weld::Button&, void);
    DECL_LINK(InputActivateHdl, weld::Entry&, bool);
    
    void SendMessage(const OUString& message);

public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        SfxBindings* pBindings);

    AIAssistantPanel(
        weld::Widget* pParent,
        SfxBindings* pBindings);
    
    virtual ~AIAssistantPanel() override;
};

} // end of namespace sw::sidebar

#endif