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



#ifndef SD_TAB_CONTROL_HXX
#define SD_TAB_CONTROL_HXX

#include <svtools/tabbar.hxx>
#include <svtools/transfer.hxx>

namespace sd {

/*************************************************************************
|*
|* TabControl-Klasse fuer Seitenumschaltung
|*
\************************************************************************/

class DrawViewShell;

class TabControl
    : public TabBar,
      public DragSourceHelper,
      public DropTargetHelper
{
//IAccessibility2 Implementation 2009-----
    //Solution: declare bIsMarked variable
    sal_uInt16    RrePageID;
//-----IAccessibility2 Implementation 2009
public:
    TabControl (DrawViewShell* pDrViewSh, ::Window* pParent);
    virtual ~TabControl (void);

    /** Inform all listeners of this control that the current page has been
        activated.  Call this method after switching the current page and is
        not done elsewhere (like when using page up/down keys).
    */
    void SendActivatePageEvent (void);

    /** Inform all listeners of this control that the current page has been
        deactivated.  Call this method before switching the current page and
        is not done elsewhere (like when using page up/down keys).
    */
    void SendDeactivatePageEvent (void);

protected:
    DrawViewShell*  pDrViewSh;
    bool                bInternalMove;

    // TabBar
    virtual void        Select();
    virtual void        DoubleClick();
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);

    virtual void        Command(const CommandEvent& rCEvt);

    virtual long        StartRenaming();
    virtual long        AllowRenaming();
    virtual void        EndRenaming();

    virtual void        ActivatePage();
    virtual long        DeactivatePage();

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

                        DECL_LINK(DeactivatePageHdl, void*);

private:
    // nested class to implement the TransferableHelper
    class TabControlTransferable : public TransferableHelper
    {
    public:
        TabControlTransferable( TabControl& rParent ) :
            mrParent( rParent ) {}
    private:

        TabControl&     mrParent;

        virtual             ~TabControlTransferable();

        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        DragFinished( sal_Int8 nDropAction );

    };

    friend class TabControl::TabControlTransferable;

    void                DragFinished( sal_Int8 nDropAction );

private:
    using TabBar::StartDrag;
};

} // end of namespace sd

#endif

