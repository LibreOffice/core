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



#ifndef _SWERROR_H
#define _SWERROR_H

#define ERR_SWGBASE 25000

/*** Error Codes **********************************************************/
#define ERR_VAR_IDX                 (ERR_SWGBASE+ 0)
#define ERR_OUTOFSCOPE              (ERR_SWGBASE+ 1)

/* Error Codes fuer Numerierungsregeln */
#define ERR_NUMLEVEL                (ERR_SWGBASE+ 2)

/* Error Codes fuer TxtNode */
#define ERR_NOHINTS                 (ERR_SWGBASE+ 3)

// von _START bis _END sind in der COREDL.DLL entsprechende Strings
// abgelegt, die erfragt werden koennen.
#define ERR_SWGMSG_START            (ERR_VAR_IDX)
#define ERR_SWGMSG_END              (ERR_NOHINTS)



#endif
