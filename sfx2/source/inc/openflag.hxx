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


#ifndef _SFX_OPENFLAG_HXX
#define _SFX_OPENFLAG_HXX

// Datei zum Bearbeiten "offnen, anschliessend funktioniert nur noch
// die dritte Variante (Lesen einer Kopie)
#define SFX_STREAM_READWRITE  (STREAM_READWRITE |  STREAM_SHARE_DENYWRITE)
// Ich arbeite roh auf dem Original, keine Kopie
// -> Datei kann anschliessend nicht zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY   (STREAM_READ | STREAM_SHARE_DENYWRITE) // + !bDirect
// Jemand anders bearbeitet das File, es wird eine Kopie erstellt
// -> Datei kann anschliessend zum Bearbeiten ge"offnet werden
#define SFX_STREAM_READONLY_MAKECOPY   (STREAM_READ | STREAM_SHARE_DENYNONE)


#endif
