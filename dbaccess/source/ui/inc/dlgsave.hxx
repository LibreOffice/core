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



#ifndef DBAUI_DLGSAVE_HXX
#define DBAUI_DLGSAVE_HXX

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
        class XHierarchicalNameAccess;
    }
    namespace sdbc {
        class XDatabaseMetaData;
        class XConnection;
    }
}}}


#define SAD_DEFAULT                 0x0000
#define SAD_ADDITIONAL_DESCRIPTION  0x0001

#define SAD_TITLE_STORE_AS          0x0000
#define SAD_TITLE_PASTE_AS          0x0100
#define SAD_TITLE_RENAME            0x0200

class Button;
class Edit;
namespace dbaui
{
    class OSaveAsDlgImpl;
    class IObjectNameCheck;
    class OSaveAsDlg : public ModalDialog
    {
    private:
        OSaveAsDlgImpl* m_pImpl;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
    public:
        OSaveAsDlg( Window * pParent,const sal_Int32& _rType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                    const String& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);

        OSaveAsDlg( Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const String& _rDefault,
                    const String& _sLabel,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);
        virtual ~OSaveAsDlg();

        String getName() const;
        String getCatalog() const;
        String getSchema() const;
    private:
        DECL_LINK(ButtonClickHdl, Button *);
        DECL_LINK(EditModifyHdl,  Edit * );

        void implInitOnlyTitle(const String& _rLabel);
        void implInit();
    };
}

#endif // DBAUI_DLGSAVE_HXX



