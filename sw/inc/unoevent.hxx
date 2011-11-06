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


#ifndef _UNOEVENT_HXX
#define _UNOEVENT_HXX

#include <svtools/unoevent.hxx>
#include <svl/macitem.hxx>


class SvxMacroItem;
class SvxMacro;
class SwXFrame;
class SwXTextFrame;
class SwXTextGraphicObject;
class SwXTextEmbeddedObject;
class SwXFrameStyle;
class SwFmtINetFmt;



class SwHyperlinkEventDescriptor : public SvDetachedEventDescriptor
{
    const ::rtl::OUString sImplName;

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
protected:
    virtual ~SwHyperlinkEventDescriptor();
public:

     SwHyperlinkEventDescriptor();



    void copyMacrosFromINetFmt(const SwFmtINetFmt& aFmt);
    void copyMacrosIntoINetFmt(SwFmtINetFmt& aFmt);

    void copyMacrosFromNameReplace(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameReplace> & xReplace);
};



// SwEventDescriptor for
// 1) SwXTextFrame
// 2) SwXGraphicObject
// 3) SwXEmbeddedObject
// All these objects are an SwXFrame, so they can use a common implementation
class SwFrameEventDescriptor : public SvEventDescriptor
{
    ::rtl::OUString sSwFrameEventDescriptor;

    SwXFrame& rFrame;

public:
    SwFrameEventDescriptor( SwXTextFrame& rFrameRef );
    SwFrameEventDescriptor( SwXTextGraphicObject& rGraphicRef );
    SwFrameEventDescriptor( SwXTextEmbeddedObject& rObjectRef );

    ~SwFrameEventDescriptor();

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
    virtual sal_uInt16 getMacroItemWhich() const;
};

class SwFrameStyleEventDescriptor : public SvEventDescriptor
{
    ::rtl::OUString sSwFrameStyleEventDescriptor;

    SwXFrameStyle& rStyle;

public:
    SwFrameStyleEventDescriptor( SwXFrameStyle& rStyleRef );

    ~SwFrameStyleEventDescriptor();

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
    virtual sal_uInt16 getMacroItemWhich() const;
};


#endif
