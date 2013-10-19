// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "mlo_uno.h"
#import "MLOCommon.h"
//#include <stdlib.h>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#undef TimeValue
#include <osl/process.h>

#include "cppuhelper/bootstrap.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XDispatchHelper.hpp"
#include <com/sun/star/frame/DispatchHelper.hpp>
#include "com/sun/star/frame/XComponentLoader.hpp"
#include "com/sun/star/frame/XStorable.hpp"
#include "com/sun/star/awt/XScrollBar.hpp"
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::view;
using namespace com::sun::star::util;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::text;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace cppu;


#define OU(C_STRING)  OUString::createFromAscii(C_STRING)

#define LOG_EVAL(REF,NS_STRING) REF.get() ? NSLog(@"%@ created",NS_STRING) : NSLog(@"%@ is null",NS_STRING)

// singletons

Reference<XComponentContext> openedComponentContext;
Reference<XMultiComponentFactory> openedXComponentFactory;
Reference<XInterface> openedXDesktop;
Reference < XComponentLoader > openedXComponentLoader;

// document singletons

Reference<XComponent> openedXComponent;
Reference<XModel> openedXModel;
Reference<XTextViewCursorSupplier> openedXTextViewCursorSupplier;
Reference<XController> openedXController;
Reference<XPageCursor> openedXPageCursor;
Reference<XViewDataSupplier> openedXViewDataSupplier;
Reference<XSearchable> openedXSearchable;
Reference<XSelectionSupplier> openedXSelectionSupplier;
Reference<XFrame> openedXFrame;
Reference<XDispatchProvider> openedXDispatchProvider;
Reference<XStorable> openedXStorable;

Reference<XIndexAccess> currentFindAll;
rtl::OUString lastSearch;
::sal_Int32 lastSearchIndex =-1;

/// functions

Reference< XComponentContext > getXComponentContext(){
    if(!openedComponentContext.get()){
        openedComponentContext.set(cppu::defaultBootstrap_InitialComponentContext());
    
        LOG_EVAL(openedComponentContext,@"XComponentContext");
    }
    return openedComponentContext;
}

Reference<XMultiComponentFactory> getXComponentFactory(){
    if(!openedXComponentFactory.get()){
        openedXComponentFactory.set(getXComponentContext()->getServiceManager());
    
        LOG_EVAL(openedXComponentFactory,@"XMultiComponentContext");
    }
  
    return openedXComponentFactory;
}

Reference<XInterface> getXDesktop(){
    if(!openedXDesktop.get()){
                
        openedXDesktop.set(getXComponentFactory()->createInstanceWithContext ( OU("com.sun.star.frame.Desktop"), getXComponentContext()));
    
        LOG_EVAL(openedXDesktop, @"XDesktop");
    }
   
    return openedXDesktop;
}

Reference <XComponentLoader> getXComponentLoader(){
    if(!openedXComponentLoader.get()){
        openedXComponentLoader.set(Reference < XComponentLoader >( Desktop::create(getXComponentContext())));
        
        LOG_EVAL(openedXComponentLoader, @"XComponentLoader");
    }
  
    return openedXComponentLoader;
}

Reference<XModel> getXModel(){
    if(!openedXModel.get()){
        Reference<XModel> model(openedXComponent.get(), UNO_QUERY);
    
        openedXModel.set(model);
    
        LOG_EVAL(openedXModel, @"XModel");
    }
    return openedXModel;
}

Reference<XController> getXController(){
   if(!openedXController.get()){
       
       openedXController.set(getXModel()->getCurrentController());
    
       LOG_EVAL(openedXController, @"XController");
    
   }
    return openedXController;
}

Reference<XTextViewCursorSupplier> getXTextViewCursorSupplier(){
    if(!openedXTextViewCursorSupplier.get()){
    
        Reference<XTextViewCursorSupplier> supplier(getXController(), UNO_QUERY);
        openedXTextViewCursorSupplier.set(supplier);
    
        LOG_EVAL(openedXTextViewCursorSupplier, @"XTextViewCursorSupplier");
    }
    return openedXTextViewCursorSupplier;
}

Reference<XPageCursor> getXPageCursor(){
    if(!openedXPageCursor.get()){
        Reference<XPageCursor> cursor(getXTextViewCursorSupplier()->getViewCursor(), UNO_QUERY);
        openedXPageCursor.set(cursor);
        
        LOG_EVAL(openedXPageCursor, @"XPageCursor");
    }
    return openedXPageCursor;
}

Reference<XViewDataSupplier> getXViewDataSupplier(){
    //if(!openedXViewDataSupplier.get()){
        Reference<XViewDataSupplier> supplier(getXController(),UNO_QUERY);
    
    openedXViewDataSupplier.set(supplier);
    
    LOG_EVAL(openedXViewDataSupplier,@"XViewDataSupplier");
    //}

    
    return openedXViewDataSupplier;
}

Reference<XSearchable> getXSearchable(){
    if(!openedXSearchable.get()){
        Reference<XSearchable> xSearchable(getXModel(),UNO_QUERY);
        openedXSearchable.set(xSearchable);
        
        LOG_EVAL(openedXSearchable,@"XSearchable");
    }
    return openedXSearchable;
}

Reference<XSelectionSupplier> getXSelectionSupplier(){
    if(!openedXSelectionSupplier.get()){
        Reference<XSelectionSupplier> supplier(getXController(),UNO_QUERY);
        openedXSelectionSupplier.set(supplier);
        
        LOG_EVAL(openedXSelectionSupplier, @"XSelectionSupplier");
    }
    return openedXSelectionSupplier;
}

Reference<XFrame> getXFrame(){
    if(!openedXFrame.get()){
        openedXFrame.set(getXController()->getFrame());
        LOG_EVAL(openedXFrame, @"XFrame");
    }
    return openedXFrame;
}

Reference<XDispatchProvider> getXDispatchProvider(){
    if(!openedXDispatchProvider.get()){
        Reference<XDispatchProvider> provider(getXFrame(),UNO_QUERY);
        openedXDispatchProvider.set(provider);
        LOG_EVAL(openedXDispatchProvider, @"XDispatchProvider");
    }
    return openedXDispatchProvider;
}

Reference<XDispatchHelper> getXDispatchHelper(){
    Reference<XDispatchHelper> helper = DispatchHelper::create(getXComponentContext());
    LOG_EVAL(helper, @"XDispatchHelper");
    return helper;
}

Reference<XStorable> getXStorable(){
    if(!openedXStorable.get()){
        Reference<XStorable> stroable(getXModel(),UNO_QUERY);
        openedXStorable.set(stroable);
        LOG_EVAL(openedXStorable, @"XDispatchProvider");
    }
    return openedXStorable;
}

NSString * createFileUri(NSString * file){
    if(file==nil){
        NSString * app_root_escaped = [[[NSBundle mainBundle] bundlePath] stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];
        file = [app_root_escaped stringByAppendingPathComponent:@"test1.odt"];
    } else {
        file = [file stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    }
    static NSString * prefix = @"file://";
    
    if(![file hasPrefix:prefix]){
        return [prefix stringByAppendingString:file];
    }
    return file;
}

void postOpenUnoInitialization(){
    getXController();
    getXSearchable();
}


extern "C"
BOOL
mlo_open_file(NSString* filepath){
    
    static const OUString TARGET = OU("_default");
    static const Sequence<PropertyValue> PROPS (0);
    
    filepath = createFileUri(filepath);
    NSLog(@"LO : Opening the file : %@",filepath );
    
    NSDate * loadStart = [NSDate date];
    getXDesktop();

    openedXComponent.set(getXComponentLoader()->loadComponentFromURL(OU([filepath UTF8String]),TARGET,0,PROPS));

    if(openedXComponent.get()){
        NSLog(@"LO : XComponent opened succesffully. Duration: %f",[[NSDate date] timeIntervalSinceDate:loadStart]);
        postOpenUnoInitialization();
        NSLog(@"LO : created document singletons");
        return YES;
    }
    
    return NO;
}

void setLastSearch(OUString string){
    lastSearch = string;
    lastSearchIndex = -1;
    currentFindAll.clear();
}

void resetLastSearch(){
    setLastSearch(OUString());
}

void convert(const OUString & source,NSMutableString * target){
    
    OString asOString = OUStringToOString( source, RTL_TEXTENCODING_ASCII_US );
    
    [target setString:[NSString stringWithUTF8String:asOString.getStr()]];
}

BOOL isResetSearch(NSString * toSearch){
    if(!toSearch || ([toSearch length] ==0)){
        resetLastSearch();
        return YES;
    }
    return NO;
}

void markFind(int searchIndex){
    Any currentFind= currentFindAll.get()->getByIndex(searchIndex);
    
    getXSelectionSupplier()->select(currentFind);
}

void markDeltaFind(int findCount,int delta){
    
    lastSearchIndex += delta;
    if(lastSearchIndex == findCount){
        lastSearchIndex=0;
    }else if(lastSearchIndex < 0){
        lastSearchIndex = findCount-1;
    }
    markFind(lastSearchIndex);
}

void markAllFinds(int findCount){
    for (int i = 0; i<findCount; i++) {
        markFind(i);
    }
}

void setCurrentFindAll(NSString * toSearch){
    const OUString current = OU([toSearch UTF8String]);
    
    if(!lastSearch.equals(current)){
        
        setLastSearch(current);
        Reference<XSearchDescriptor> descriptor = getXSearchable()->createSearchDescriptor();
        descriptor->setSearchString(current);
        currentFindAll.set(getXSearchable()->findAll(descriptor));
    }
}

extern "C"
void
mlo_close(void){
    
    if(openedXComponent.get()){
        
        NSLog(@"Disposing of the XComponent for the opened document");

        openedXComponent->dispose();
        openedXComponent.clear();
        openedXModel.clear();
        openedXTextViewCursorSupplier.clear();
        openedXController.clear();
        openedXPageCursor.clear();
        openedXViewDataSupplier.clear();
        openedXSearchable.clear();
        openedXSelectionSupplier.clear();
        openedXFrame.clear();
        openedXDispatchProvider.clear();
        resetLastSearch();
        
        NSLog(@"XComponent disposed successfully");
    }
}

long getLong(Reference<XPropertySet> & set,const OUString & name){
    
    if(set.get()){
    
        return set->getPropertyValue(name).get<long>();
    }
    return -666;
    
}


int getInt(Reference<XPropertySet> & set,const OUString & name){
    
    return getLong(set, name);
    
}


extern "C"
void mlo_fetch_view_data(NSMutableString * mutableString){
    
    Any any = getXController()->getViewData();
    
    OUString ou = any.get<OUString>();
    
    convert(ou, mutableString);
}

extern "C"
long
mlo_get_pages(BOOL last){
    
    if(last){
        
        static const OUString PAGE_COUNT = OU("PageCount");
        
        Reference< XPropertySet > set(  getXController(), UNO_QUERY );
        return getLong(set,PAGE_COUNT);
    }
    
    
    return getXPageCursor()->getPage();
}


extern "C"
int
mlo_find(NSString * toSearch,MLOFindSelectionType type){

    if(!isResetSearch(toSearch)){
        
        setCurrentFindAll(toSearch);
        
        if(currentFindAll.get()){
            sal_Int32 count =currentFindAll.get()->getCount();
            if(count>0){
                switch (type) {
                    case MARK_ALL:
                        // mark all returns total count;
                        markAllFinds(count);
                        return  count;
                    case MARK_FIRST:
                        // mark first returns total count
                        // index will be zero
                        markDeltaFind(count,1);
                        return count;
                    case MARK_NEXT:
                        // mark next returns the index
                        markDeltaFind(count, 1);
                        return lastSearchIndex;
                    case MARK_PREVIOUS:
                        // mark previous returns the last index
                        markDeltaFind(count, -1);
                        return lastSearchIndex;
                }
            }
        }
    }
    return -1;
}

extern "C"
void
mlo_get_selection(NSMutableString * mutableString){
    OUString asOuString;
    
    Reference<XInterface> currentSelection = getXModel()->getCurrentSelection();
    
    if(currentSelection.get()){
    
        Reference< XIndexAccess > xIndexAccess( currentSelection, UNO_QUERY );

        if(xIndexAccess.get() && (xIndexAccess->getCount()>0)){
            Reference< XTextRange > xTextRange(xIndexAccess->getByIndex(0),UNO_QUERY);
            if(xTextRange.get()){
                asOuString=xTextRange->getString();
            }
        }
    }
    
    convert(asOuString, mutableString);
}

extern "C"
BOOL
mlo_is_document_open(void){
    return openedXComponent.is() ? YES : NO;
}

extern "C"
void
mlo_select_all(void){
    getXDispatchHelper()->executeDispatch(getXDispatchProvider(),
                                          OUString(".uno:SelectAll"),
                                          OUString("_self"),
                                          0,
                                          Sequence<PropertyValue >());
}

extern "C"
void
mlo_save(void){
    if(mlo_is_document_open()){
        Reference<XStorable> storable =getXStorable();
        if(storable->isReadonly()){
            NSLog(@"Cannot save changes. File is read only");
        }else{
            storable->store();
            NSLog(@"saved changes");
        }
    }
}