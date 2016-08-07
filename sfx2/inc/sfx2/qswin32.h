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



#ifndef _QSWIN32_H
#define _QSWIN32_H

#define QUICKSTART_CLASSNAMEA           "SO Listener Class"
#define QUICKSTART_WINDOWNAMEA          "SO Listener Window"
#define SHUTDOWN_QUICKSTART_MESSAGEA    "SO KillTray"

#define QUICKSTART_CLASSNAMEW           L##QUICKSTART_CLASSNAMEA
#define QUICKSTART_WINDOWNAMEW          L##QUICKSTART_WINDOWNAMEA
#define SHUTDOWN_QUICKSTART_MESSAGEW    L##SHUTDOWN_QUICKSTART_MESSAGEA

#ifdef UNICODE
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEW
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEW
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEW
#else
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEA
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEA
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEA
#endif

#endif /* _QSWIN32_H */
