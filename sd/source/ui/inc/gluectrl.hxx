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



#ifndef SD_GLUECTRL_HXX
#define SD_GLUECTRL_HXX

#include <vcl/lstbox.hxx>
#include <sfx2/tbxctrl.hxx>

/*************************************************************************
|*
|* GluePointEscDirLB
|*
\************************************************************************/
class GlueEscDirLB : public ListBox
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
public:
                GlueEscDirLB( Window* pParent,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame );
                 ~GlueEscDirLB();

    virtual void Select();

    void         Fill();
};

/*************************************************************************
|*
|* Toolbox-Controller fuer Klebepunkte-Austrittsrichtung
|*
\************************************************************************/

class SdTbxCtlGlueEscDir: public SfxToolBoxControl
{
private:
    sal_uInt16  GetEscDirPos( sal_uInt16 nEscDir );

public:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlGlueEscDir( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
            ~SdTbxCtlGlueEscDir() {}
};

#endif // SD_GLUECTRL_HXX

