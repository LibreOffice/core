#include <officelabs/AgentConnection.hxx>

namespace officelabs {

AgentConnection::AgentConnection()
    : m_connected(false)
{
    // Minimal stub - just set connected to true for now
    m_connected = true;
}

AgentConnection::~AgentConnection() {
}

bool AgentConnection::checkConnection() {
    // Minimal stub - always return true
    return true;
}

AgentResponse AgentConnection::sendMessage(const OUString& message, const OUString& documentContext) {
    // Minimal stub implementation
    AgentResponse response;
    response.message = "Echo: " + message;
    response.hasPatch = false;
    return response;
}

} // namespace officelabs