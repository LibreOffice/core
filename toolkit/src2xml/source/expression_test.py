#!/usr/bin/env python
# *************************************************************
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
# *************************************************************


import sys
import expression

def run (exp):
    tokens = exp.split()
    expparser = expression.ExpParser(tokens)
    expparser.build()
    expparser.dumpTree()

def main ():
    run("6 + 34")
    run("6 + 34 - 10")
    run("6 + 34 - 10 + 200")
    run("6 + 34 - 10 * 200")
    run("6 + 34 - 10 * 200 + 18")
    run("6 + 34 - 10 * 200 + 18 / 2")

    run("6 * ( ( 10 + 2 ) - 10 ) * 33")

if __name__ == '__main__':
    main()
