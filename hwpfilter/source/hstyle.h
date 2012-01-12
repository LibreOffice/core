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



/* $Id: hstyle.h,v 1.3 2008-04-10 12:07:29 rt Exp $ */

#ifndef _HWPSTYLE_H_
#define _HWPSTYLE_H_

#include <stdlib.h>
#include <string.h>

#include <hwplib.h>
#include <hinfo.h>
/**
 * @short Using for global style object like "Standard"
 */
class DLLEXPORT HWPStyle
{
    short nstyles;
    void *style;
    public:
        HWPStyle( void );
        ~HWPStyle( void );

        int Num( void ) const;
        char *GetName( int n ) const;
        CharShape *GetCharShape( int n ) const;
        ParaShape *GetParaShape( int n ) const;

        void SetName( int n, char *name );
        void SetCharShape( int n, CharShape *cshapep );
        void SetParaShape( int n, ParaShape *pshapep );

        bool Read( HWPFile &hwpf );
};
#endif
/* _HWPSTYLE+H_ */
