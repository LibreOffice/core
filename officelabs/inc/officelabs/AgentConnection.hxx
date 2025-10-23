#ifndef INCLUDED_OFFICELABS_AGENTCONNECTION_HXX
#define INCLUDED_OFFICELABS_AGENTCONNECTION_HXX

#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>

namespace officelabs {

struct AgentResponse {
    OUString message;
    bool hasPatch;
};

class SFX2_DLLPUBLIC AgentConnection {
private:
    bool m_connected;

public:
    AgentConnection();
    ~AgentConnection();
    
    bool checkConnection();
    bool isConnected() const { return m_connected; }
    
    AgentResponse sendMessage(const OUString& message, const OUString& documentContext);
};

} // namespace officelabs

#endif