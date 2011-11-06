/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
