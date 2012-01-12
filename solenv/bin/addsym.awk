#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

# Add certain symbol patterns to the first global section.
#
# The below code fails with 'perverted' mapfiles (using a strange line layout,
# or containing version UDK_3_0_0 without a global section, ...).

BEGIN { state = 0 }
END {
    if (state == 0) {
        print "# Weak RTTI symbols for C++ exceptions:"
        print "UDK_3_0_0 {"
        print "global:"
        print "_ZTI*; _ZTS*; # weak RTTI symbols for C++ exceptions"
        if (ENVIRON["USE_SYSTEM_STL"] != "YES")
            print "_ZN4_STL7num_put*; # for STLport"
        print "};"
    }
}
state == 2 {
    print "_ZTI*; _ZTS*; # weak RTTI symbols for C++ exceptions"
    if (ENVIRON["USE_SYSTEM_STL"] != "YES")
        print "_ZN4_STL7num_put*; # for STLport"
    state = 3
}
# #i66636# - ???
/^[\t ]*UDK_3_0_0[\t ]*\{/ { state = 1 }
/^[\t ]*global[\t ]*:/ && state == 1 { state = 2 }
{ print }
