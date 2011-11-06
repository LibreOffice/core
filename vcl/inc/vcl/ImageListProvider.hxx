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



#ifndef _SV_IMAGELISTPROVIDER_HXX
#define _SV_IMAGELISTPROVIDER_HXX

#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace lang { class IllegalArgumentException; }}}}

class ImageList;


namespace vcl
{
    enum ImageListType
    {
        IMAGELISTTYPE_UNKNOWN = 0,
        HIGHCONTRAST_NO = 1,
        HIGHCONTRAST_YES = 2
    };

    /* abstract */ class IImageListProvider
    {
    public:
        virtual ImageList getImageList(ImageListType) SAL_THROW (( com::sun::star::lang::IllegalArgumentException )) = 0;
    };
} /* vcl */

#endif /* _SV_IMAGELISTPROVIDER_HXX */
