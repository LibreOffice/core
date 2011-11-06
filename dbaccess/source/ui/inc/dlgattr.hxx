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


#ifndef DBAUI_SBATTRDLG_HXX
#define DBAUI_SBATTRDLG_HXX


#ifndef DBAUI_SBATTRDLG_HRC
#include "dlgattr.hrc"
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif



class SvxNumberInfoItem;
class SfxItemSet;
class SvNumberFormatter;
//.........................................................................
namespace dbaui
{
//.........................................................................

    class SbaSbAttrDlg : public SfxTabDialog
    {
        String aTitle;
        SvxNumberInfoItem* pNumberInfoItem;

    public:
        SbaSbAttrDlg( Window * pParent, const SfxItemSet*, SvNumberFormatter*, sal_uInt16 nFlags = TP_ATTR_CHAR , sal_Bool bRow = sal_False);
        ~SbaSbAttrDlg();

        virtual void  PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage );
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif//DBAUI_SBATTRDLG_HXX

