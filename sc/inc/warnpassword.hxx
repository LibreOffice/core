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



#ifndef SC_WARNPASSWORD_HXX
#define SC_WARNPASSWORD_HXX



class SfxMedium;
/** Static API helper functions. */
class ScWarnPassword
{
public:
    /** Opens a query warning dialog.
     @descr Pop up Query on export  with a choice of yes/no if a
     an encrypted password is imported.
     @return true if yes and false if no. default to true.
   */
    static bool        WarningOnPassword( SfxMedium& rMedium );
};


#endif

