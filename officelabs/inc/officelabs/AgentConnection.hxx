#ifndef INCLUDED_OFFICELABS_AGENTCONNECTION_HXX
#define INCLUDED_OFFICELABS_AGENTCONNECTION_HXX

#include <officelabs/officelabsdllapi.h>
#include <rtl/ustring.hxx>
#include <string>
#include <vector>
#include <optional>

namespace officelabs {

// Command for automatic document editing
struct OFFICELABS_DLLPUBLIC AutoEditCommand {
    OUString action;     // "insert", "replace", "delete", "format", "clear_and_write"
    OUString findText;   // Text to find for replace/delete/format
    OUString newText;    // Text to insert or replace with
    OUString position;   // "cursor", "end", "start", "after_find", "before_find"
    bool bold = false;
    bool italic = false;
    bool underline = false;
    int headingLevel = 0;  // 0 = normal, 1-6 = heading levels
};

struct OFFICELABS_DLLPUBLIC AgentResponse {
    OUString message;
    bool hasPatch;
    OUString patchType;
    OUString patchTarget;
    OUString patchOldValue;
    OUString patchNewValue;
    OUString patchDiff;
    std::vector<AutoEditCommand> autoEdits;  // Commands to execute automatically
};

class OFFICELABS_DLLPUBLIC AgentConnection {
private:
    std::string m_backendUrl;
    bool m_connected;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    AgentResponse parseResponse(const std::string& json);

public:
    AgentConnection();
    ~AgentConnection();
    
    bool checkConnection();
    bool isConnected() const { return m_connected; }
    
    AgentResponse sendMessage(const OUString& message, const OUString& documentContent, const OUString& selection = OUString());
    void setBackendUrl(const OUString& url);
};

} // namespace officelabs

#endif
