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



#include    <codemaker/options.hxx>

using namespace rtl;

Options::Options()
{
}

Options::~Options()
{

}

const OString& Options::getProgramName() const
{
    return m_program;
}

sal_Bool Options::isValid(const OString& option)
{
    return (m_options.count(option) > 0);
}

const OString Options::getOption(const OString& option)
    throw( IllegalArgument )
{
    if (m_options.count(option) > 0)
    {
        return m_options[option];
    } else
    {
        throw IllegalArgument("Option is not valid or currently not set.");
    }
}

const StringVector& Options::getInputFiles()
{
    return m_inputFiles;
}

