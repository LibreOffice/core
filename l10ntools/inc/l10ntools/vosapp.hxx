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

#ifndef VOSAPP_HXX
#define VOSAPP_HXX

#include <sal/main.h>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <vos/process.hxx>

// Mininmal vcl/svapp compatibility without vcl dependence
class Application
{
public:
    sal_uInt16 GetCommandLineParamCount();
    XubString GetCommandLineParam( sal_uInt16 nParam );
    virtual void Main() = 0;
};

// Urg: Cut & Paste from svapp.cxx: we don't want to depend on vcl
sal_uInt16 Application::GetCommandLineParamCount()
{
    vos::OStartupInfo aStartInfo;
    return (sal_uInt16)aStartInfo.getCommandArgCount();
}

XubString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    vos::OStartupInfo aStartInfo;
    rtl::OUString aParam;
    aStartInfo.getCommandArg( nParam, aParam );
    return XubString( aParam );
}

#endif /* VOSAPP_HXX */
