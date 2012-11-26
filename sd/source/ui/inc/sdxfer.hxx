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



#ifndef _SD_SDXFER_HXX
#define _SD_SDXFER_HXX

#include <svtools/transfer.hxx>
#include <vcl/graph.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>

// ------------------
// - SdTransferable -
// ------------------

class SdDrawDocument;
class SdrObject;
class INetBookmark;
class ImageMap;
class VirtualDevice;

namespace sd {
class DrawDocShell;
class View;
}

class SfxObjectShellRef;
class SdTransferable : public TransferableHelper, public SfxListener
{
public:

                                    SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, bool bInitOnGetData );
                                    ~SdTransferable();

    void                            SetDocShell( const SfxObjectShellRef& rRef ) { maDocShellRef = rRef; }
    const SfxObjectShellRef&        GetDocShell() const { return maDocShellRef; }

    void                            SetWorkDocument( const SdDrawDocument* pWorkDoc ) { mpSdDrawDocument = mpSdDrawDocumentIntern = (SdDrawDocument*) pWorkDoc; }
    const SdDrawDocument*           GetWorkDocument() const { return mpSdDrawDocument; }

    void                            SetView( const ::sd::View* pView ) { mpSdView = pView; }
    const ::sd::View*                   GetView() const { return mpSdView; }

    void                            SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc );

    void                            SetStartPos( const basegfx::B2DPoint& rStartPos ) { maStartPos = rStartPos; }
    const basegfx::B2DPoint&        GetStartPos() const { return maStartPos; }

    void                            SetInternalMove( bool bSet ) { mbInternalMove = bSet; }
    bool                            IsInternalMove() const { return mbInternalMove; }

    bool                            HasSourceDoc( const SdDrawDocument* pDoc ) const { return( mpSourceDoc == pDoc ); }

    void                            SetPageBookmarks( const List& rPageBookmarks, bool bPersistent );
    bool                            IsPageTransferable() const { return mbPageTransferable; }
    bool                            HasPageBookmarks() const { return( mpPageDocShell && ( maPageBookmarks.Count() > 0 ) ); }
    const List&                     GetPageBookmarks() const { return maPageBookmarks; }
    ::sd::DrawDocShell*                 GetPageDocShell() const { return mpPageDocShell; }

    sal_Bool                        SetTableRTF( SdDrawDocument*, const ::com::sun::star::datatransfer::DataFlavor& );

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
    static SdTransferable*          getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void                    DragFinished( sal_Int8 nDropAction );
    SdDrawDocument*                 GetSourceDoc (void) const;

    /** User data objects can be used to store information temporarily
        at the transferable.  The slide sorter uses this to store
        previews of the slides that are referenced by the
        transferable.
    */
    class UserData {public:virtual~UserData(){}};

    /** Add a user data object.  When it was added before (and not
        removed) then this call is ignored.
    */
    void AddUserData (const ::boost::shared_ptr<UserData>& rpData);

    /** Remove a previously added user data object.  When the object
        was never added or removed before then this call is ignored.
    */
    void RemoveUserData (const ::boost::shared_ptr<UserData>& rpData);

    /** Return the number of user data objects.
    */
    sal_Int32 GetUserDataCount (void) const;

    /** Return the specified user data object.  When the index is not
        valid, ie not in the range [0,count) then an empty pointer is
        returned.
    */
    ::boost::shared_ptr<UserData> GetUserData (const sal_Int32 nIndex) const;

protected:

    virtual void                    AddSupportedFormats();
    virtual sal_Bool                GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool                WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void                    ObjectReleased();

    virtual sal_Int64 SAL_CALL      getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );

private:

    SfxObjectShellRef               maDocShellRef;
    ::sd::DrawDocShell*             mpPageDocShell;
    List                            maPageBookmarks;
    TransferableDataHelper*         mpOLEDataHelper;
    TransferableObjectDescriptor*   mpObjDesc;
    const ::sd::View*               mpSdView;
    ::sd::View*                     mpSdViewIntern;
    SdDrawDocument*                 mpSdDrawDocument;
    SdDrawDocument*                 mpSdDrawDocumentIntern;
    SdDrawDocument*                 mpSourceDoc;
    VirtualDevice*                  mpVDev;
    INetBookmark*                   mpBookmark;
    Graphic*                        mpGraphic;
    ImageMap*                       mpImageMap;
    Rectangle                       maVisArea;
    basegfx::B2DPoint               maStartPos;
    bool                            mbInternalMove               : 1;
    bool                            mbOwnDocument                : 1;
    bool                            mbOwnView                    : 1;
    bool                            mbLateInit                   : 1;
    bool                            mbPageTransferable           : 1;
    bool                            mbPageTransferablePersistent : 1;
    bool                            mbIsUnoObj                  : 1;
    ::std::vector<boost::shared_ptr<UserData> > maUserData;

                                    // not available
                                    SdTransferable();
                                    SdTransferable( const SdTransferable& );
    SdTransferable&                 operator=( const SdTransferable& );

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

};

#endif // _SD_SDXFER_HXX
