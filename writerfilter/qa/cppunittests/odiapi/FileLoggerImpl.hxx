/*  Copyright 2005 Sun Microsystems, Inc. */

#ifndef INCLUDED_FILELOGGERIMPL_HXX
#define INCLUDED_FILELOGGERIMPL_HXX

#include <odiapi/props/Logger.hxx>
#include <fstream>
#include <stack>

namespace util {

/** A file logger
 */
class FileLoggerImpl : public util::Logger
{
public:
    FileLoggerImpl(const std::string& fileName);

    virtual void beginTree();
    virtual void endTree();

    virtual void beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse);
    virtual void endNode(const std::string& nodeId);

private:
    std::ofstream file_;
    std::stack<std::string> nodeStack_;
};

} // namespace util

#endif // INCLUDED_LOGGER_HXX
