/*  Copyright 2005 Sun Microsystems, Inc. */

#include "FileLoggerImpl.hxx"
#include <iostream>

using namespace std;

namespace util
{

    FileLoggerImpl::FileLoggerImpl(const string& fileName) :
        file_(fileName.c_str())
    {
        if (!file_)
            throw "Cannot open file";
    }

    void FileLoggerImpl::beginTree()
    {
        file_ << "digraph {" << endl;
    }

    void FileLoggerImpl::endTree()
    {
        file_ << "}" << endl;
    }

    void FileLoggerImpl::beginNode(const std::string& nodeId, const std::string& value, const std::string& refersToNodeId, bool inUse)
    {
        if (!nodeStack_.empty())
        {
          if (inUse)
            file_ << nodeId << " [ label=\"(" << value << ")\", shape=box, color=grey, style=filled ];"<< endl;
          else
            file_ << nodeId << " [ label=\"(" << value << ")\" ];"<< endl;

          file_ << nodeStack_.top() << " -> " << nodeId << ";" << endl;

          if (!refersToNodeId.empty())
            file_ << nodeId << " -> " << refersToNodeId << " [ color=grey, weight=0 ];" << endl;
        }
        else
        {
            file_ << nodeId << " [ label=\"(" << value << ")\", shape=diamond ];"<< endl;
        }
        nodeStack_.push(nodeId);
    }

    void FileLoggerImpl::endNode(const std::string& nodeId)
    {
        nodeStack_.pop();
    }

} // namespace util

