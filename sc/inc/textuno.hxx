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



#ifndef SC_TEXTSUNO_HXX
#define SC_TEXTSUNO_HXX

#include "global.hxx"           // ScRange, ScAddress
#include "address.hxx"
#include <editeng/unotext.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>
#include "scdllapi.h"

class EditEngine;
class EditTextObject;
class SvxEditEngineForwarder;
class ScDocShell;
class ScAddress;
class ScCellObj;
class ScSimpleEditSource;
class ScSharedCellEditSource;
class ScEditEngineDefaulter;
class ScFieldEditEngine;

struct ScHeaderFieldData;


#define SC_HDFT_LEFT    0
#define SC_HDFT_CENTER  1
#define SC_HDFT_RIGHT   2


//  ScHeaderFooterContentObj ist ein dummer Container, der per setPropertyValue
//  wieder in die Seitenvorlage geschrieben werden muss

class ScHeaderFooterContentObj : public cppu::WeakImplHelper3<
                            com::sun::star::sheet::XHeaderFooterContent,
                            com::sun::star::lang::XUnoTunnel,
                            com::sun::star::lang::XServiceInfo >
{
private:
    EditTextObject* pLeftText;
    EditTextObject* pCenterText;
    EditTextObject* pRightText;
    SfxBroadcaster  aBC;

    ScHeaderFooterContentObj(); // disabled

public:
                            ScHeaderFooterContentObj( const EditTextObject* pLeft,
                                                      const EditTextObject* pCenter,
                                                      const EditTextObject* pRight );
    virtual                 ~ScHeaderFooterContentObj();

                            // fuer ScPageHFItem (per getImplementation)
    const EditTextObject*   GetLeftEditObject() const   { return pLeftText; }
    const EditTextObject*   GetCenterEditObject() const { return pCenterText; }
    const EditTextObject*   GetRightEditObject() const  { return pRightText; }

    void                    AddListener( SfxListener& rListener );
    void                    RemoveListener( SfxListener& rListener );

    void                    UpdateText( sal_uInt16 nPart, EditEngine& rSource );

                            // XHeaderFooterContent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getLeftText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getCenterText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getRightText() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFooterContentObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::sheet::XHeaderFooterContent> xObj );

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  ScHeaderFooterTextData: shared data between sub objects of a ScHeaderFooterTextObj

class ScHeaderFooterTextData : public SfxListener
{
private:
    ScHeaderFooterContentObj&   rContentObj;
    sal_uInt16                      nPart;
    ScEditEngineDefaulter*      pEditEngine;
    SvxEditEngineForwarder*     pForwarder;
    sal_Bool                        bDataValid;
    sal_Bool                        bInUpdate;

public:
                            ScHeaderFooterTextData( ScHeaderFooterContentObj& rContent,
                                                    sal_uInt16 nP );
                            ~ScHeaderFooterTextData();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // helper functions
    SvxTextForwarder*       GetTextForwarder();
    void                    UpdateData();
    ScEditEngineDefaulter*  GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    sal_uInt16                  GetPart() const         { return nPart; }
    ScHeaderFooterContentObj& GetContentObj() const { return rContentObj; }
};

//  ScHeaderFooterTextObj veraendert den Text in einem ScHeaderFooterContentObj

class ScHeaderFooterTextObj : public cppu::WeakImplHelper5<
                            com::sun::star::text::XText,
                            com::sun::star::text::XTextRangeMover,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::text::XTextFieldsSupplier,
                            com::sun::star::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData      aTextData;
    SvxUnoText*                 pUnoText;

    void                    CreateUnoText_Impl();

public:
                            ScHeaderFooterTextObj( ScHeaderFooterContentObj& rContent,
                                                    sal_uInt16 nP );
    virtual                 ~ScHeaderFooterTextObj();

    const SvxUnoText&       GetUnoText();

    static void             FillDummyFieldData( ScHeaderFieldData& rData );

                            // XText
    virtual void SAL_CALL   insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeTextContent( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextContent >& xContent )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setString( const ::rtl::OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XTextRangeMover
    virtual void SAL_CALL   moveTextRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nParagraphs )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XTextFieldsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


//  derived cursor objects for getImplementation and getText/getStart/getEnd

//! uno3: SvxUnoTextCursor is not derived from XUnoTunnel, but should be (?)

class ScCellTextCursor : public SvxUnoTextCursor
{
    ScCellObj&              rTextObj;

public:
                            ScCellTextCursor(const ScCellTextCursor& rOther);
                            ScCellTextCursor(ScCellObj& rText);
        virtual                                 ~ScCellTextCursor() throw();

    ScCellObj&              GetCellObj() const  { return rTextObj; }

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};

class ScHeaderFooterTextCursor : public SvxUnoTextCursor
{
private:
    ScHeaderFooterTextObj&  rTextObj;

public:
                            ScHeaderFooterTextCursor(const ScHeaderFooterTextCursor& rOther);
                            ScHeaderFooterTextCursor(ScHeaderFooterTextObj& rText);
        virtual                                 ~ScHeaderFooterTextCursor() throw();

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScHeaderFooterTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};

class ScDrawTextCursor : public SvxUnoTextCursor
{
private:
    com::sun::star::uno::Reference< com::sun::star::text::XText > xParentText;

public:
                            ScDrawTextCursor(const ScDrawTextCursor& rOther);
                            ScDrawTextCursor( const com::sun::star::uno::Reference<
                                                com::sun::star::text::XText >& xParent,
                                            const SvxUnoTextBase& rText );
    virtual                  ~ScDrawTextCursor() throw();

                            // SvxUnoTextCursor methods reimplemented here:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScDrawTextCursor* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );
};


// ScAnnotationTextCursor isn't needed anymore - SvxUnoTextCursor is used instead


//  ScEditEngineTextObj for formatted cell content that is not inserted in a cell or header/footer
//  (used for XML export of change tracking contents)

class ScSimpleEditSourceHelper
{
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    ScSimpleEditSource*     pOriginalSource;

public:
            ScSimpleEditSourceHelper();
            ~ScSimpleEditSourceHelper();

    ScSimpleEditSource* GetOriginalSource() const   { return pOriginalSource; }
    ScEditEngineDefaulter* GetEditEngine() const    { return pEditEngine; }
};

class ScEditEngineTextObj : public ScSimpleEditSourceHelper, public SvxUnoText
{
public:
                        ScEditEngineTextObj();
        virtual                         ~ScEditEngineTextObj() throw();

    void                SetText( const EditTextObject& rTextObject );
    EditTextObject*     CreateTextObject();
};


//  ScCellTextData: shared data between sub objects of a cell text object

class ScCellTextData : public SfxListener
{
protected:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScFieldEditEngine*      pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    ScSharedCellEditSource* pOriginalSource;
    sal_Bool                    bDataValid;
    sal_Bool                    bInUpdate;
    sal_Bool                    bDirty;
    sal_Bool                    bDoUpdate;

protected:
    virtual void            GetCellText(const ScAddress& rCellPos, String& rText);

public:
                            ScCellTextData(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellTextData();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // helper functions for ScSharedCellEditSource:
    virtual SvxTextForwarder* GetTextForwarder();
    void                    UpdateData();
    ScFieldEditEngine*      GetEditEngine() { GetTextForwarder(); return pEditEngine; }

    ScSharedCellEditSource* GetOriginalSource();        // used as argument for SvxUnoText ctor

                            // used for ScCellEditSource:
    ScDocShell*             GetDocShell() const     { return pDocShell; }
    const ScAddress&        GetCellPos() const      { return aCellPos; }

    void                    SetDirty(sal_Bool bValue)   { bDirty = bValue; }
    sal_Bool                    IsDirty() const         { return bDirty; }
    void                    SetDoUpdate(sal_Bool bValue)    { bDoUpdate = bValue; }
};

class ScCellTextObj : public ScCellTextData, public SvxUnoText
{
public:
                ScCellTextObj(ScDocShell* pDocSh, const ScAddress& rP);
        virtual         ~ScCellTextObj() throw();
};


#endif

