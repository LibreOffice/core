/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs AI Assistant Panel
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_AIASSISTANTPANEL_HXX

#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/TextView.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <officelabs/AgentConnection.hxx>

class SfxBindings;
class SwWrtShell;

namespace sw::sidebar {

class AIAssistantPanel : public PanelLayout
{
private:
    std::unique_ptr<weld::Label> m_xStatusLabel;
    std::unique_ptr<weld::TextView> m_xChatHistory;
    std::unique_ptr<weld::Entry> m_xInputField;
    std::unique_ptr<weld::Button> m_xSendButton;
    std::unique_ptr<weld::Button> m_xInsertButton;

    SfxBindings* m_pBindings;
    SwWrtShell* m_pWrtShell;

    // Backend connection
    std::unique_ptr<officelabs::AgentConnection> m_pAgentConnection;

    // State tracking
    bool m_bProcessing;
    OUString m_sLastAIResponse;

    DECL_LINK(SendClickHdl, weld::Button&, void);
    DECL_LINK(InputActivateHdl, weld::Entry&, bool);
    DECL_LINK(InsertClickHdl, weld::Button&, void);

    void SendMessage(const OUString& message);
    void UpdateStatus(const OUString& status);
    void AppendToChat(const OUString& sender, const OUString& message);
    OUString GetDocumentContext();
    void ProcessResponse(const officelabs::AgentResponse& response);

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