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



#ifndef VCL_IMAGEREPOSITORY_HXX
#define VCL_IMAGEREPOSITORY_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>

class BitmapEx;
class Image;

namespace vcl
{

    /**
        provides access to the application's image repository
        (packed images and brand images)
     */
    class VCL_DLLPUBLIC ImageRepository
    {
    public:
        /** loads an image from the application's image repository
            @param  _rName
                the name of the image to load.
            @param  _out_rImage
                will take the image upon successful return.
            @param  bSearchLanguageDependent
                determines whether a language-dependent image is to be searched.
            @return
                whether or not the image could be loaded successfully.
        */
        static bool loadImage(
            const ::rtl::OUString& _rName,
            BitmapEx& _out_rImage,
            bool bSearchLanguageDependent
        );

        /** load an image from the application's branding directory

            @param rName
                the name of the image to load, without extension
            @param rImage
                will take the image upon successful return.
            @param bIgnoreHighContrast
                if true, high contrast mode is not taken into account when
                searching for the image
            @param  bSearchLanguageDependent
                determines whether a language-dependent image is to be searched.
         */
        static bool loadBrandingImage(
            const rtl::OUString &rName,
            Image &rImage,
            bool bSearchLanguageDependent = false
        );
    };
}

#endif // VCL_IMAGEREPOSITORY_HXX

