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


#ifndef _DBTABLEPREVIEWDIALOG_HXX
#define _DBTABLEPREVIEWDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <com/sun/star/uno/Sequence.h>

namespace com{ namespace sun{ namespace star{
    namespace beans{  struct PropertyValue; }
    namespace frame{ class XFrame;     }
    }}}


/*-- 08.04.2004 14:04:29---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwDBTablePreviewDialog : public SfxModalDialog
{
    FixedInfo       m_aDescriptionFI;
    Window*         m_pBeamerWIN;
    OKButton        m_aOK;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >         m_xFrame;
public:
    SwDBTablePreviewDialog(Window* pParent,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rValues  );
    ~SwDBTablePreviewDialog();

};
#endif

