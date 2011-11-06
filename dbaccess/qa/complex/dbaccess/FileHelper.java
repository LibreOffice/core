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



package complex.dbaccess;

public class FileHelper
{
    private FileHelper(){}
    static public String getOOoCompatibleFileURL( String _javaFileURL )
    {
        String returnURL = _javaFileURL;
        if ( ( returnURL.indexOf( "file:/" ) == 0 ) && ( returnURL.indexOf( "file:///" ) == -1 ) )
        {
            // for some reason, the URLs here in Java start with "file:/" only, instead of "file:///"
            // Some of the office code doesn't like this ...
            returnURL = "file:///" + returnURL.substring( 6 );
        }
        return returnURL;
    }
}
