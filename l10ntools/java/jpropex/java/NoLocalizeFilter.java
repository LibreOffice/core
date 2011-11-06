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




import java.io.*;
import java.util.regex.*;

// exit if the sequence x-no-localize is found in stream!
public class NoLocalizeFilter extends FilterInputStream
{
    InputStream in;
    Pattern p = Pattern.compile("#[\\s]*x-no-translate");

    public NoLocalizeFilter( InputStream in ) {
        super(in);
        this.in = in;
    }
    public int read(byte[] b, int off, int len) throws IOException
    {
        String search = new String( b );
        Matcher m = p.matcher( search );
        if( m.find() )
        {
            in.close();
            close();
            System.exit( 0 );
        }
        return in.read( b , off , len );
    }
}
