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



#ifndef _SD_TEXTAPI_HXX_
#define _SD_TEXTAPI_HXX_

#include <editeng/unoedsrc.hxx>
#include <editeng/unotext.hxx>
#include <editeng/eeitem.hxx>
#include <rtl/ref.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoipset.hxx>

class SdDrawDocument;

namespace sd {

class TextAPIEditSource;

class TextApiObject : public SvxUnoText
{
public:
    static rtl::Reference< TextApiObject > create( SdDrawDocument* pDoc );

    virtual             ~TextApiObject() throw();

    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    OutlinerParaObject* CreateText();
    void                SetText( OutlinerParaObject& rText );
    String              GetText();

    static TextApiObject* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& );

private:
    TextAPIEditSource*  mpSource;
    TextApiObject( TextAPIEditSource* pEditSource );
};

} // namespace sd


#endif // _SD_TEXTAPI_HXX_
