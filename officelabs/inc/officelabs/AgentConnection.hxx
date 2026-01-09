#ifndef INCLUDED_OFFICELABS_AGENTCONNECTION_HXX
#define INCLUDED_OFFICELABS_AGENTCONNECTION_HXX

#include <rtl/ustring.hxx>
#include <string>
#include <optional>

namespace officelabs {

struct AgentResponse {
    OUString message;
    bool hasPatch;
    OUString patchType;
    OUString patchTarget;
    OUString patchOldValue;
    OUString patchNewValue;
    OUString patchDiff;
};

class AgentConnection {
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
    
    AgentResponse sendMessage(const OUString& message, const OUString& documentContext);
    void setBackendUrl(const OUString& url);
};

} // namespace officelabs

#endif
