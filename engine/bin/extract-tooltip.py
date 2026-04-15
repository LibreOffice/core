#!/usr/bin/env python
import sys
import os
import re
import urlparse

def usage():
    message = """ usage: {program} inDir outDir
inDir: directory containing .ht files
outDir: target for the new files"""
    print(message.format(program = os.path.basename(sys.argv[0])))

def parseFile(filename):
    file = open(filename, "r")
    data = file.readlines()
    data = [line.rstrip('\n') for line in data]

    pairs = {}
    regEx = re.compile(r"^(\S+)\s(\S+)\s(\S+)\s((?:\s*\S*)+)$")
    old_line = None
    for line in data:
        if len(line) > 0:
            if old_line is not None:
                print(filename)
                # print("failed to parse line")
                # print(old_line)
                line = old_line + line
                print(line)
                old_line = None
            split_line = regEx.split(line)
            # print(split_line)
            # print(urlparse.unquote(split_line[2]))
            # print(split_line[4])
            if (old_line is None and split_line[4] == "" and split_line[3] != "0"):
                print(line)
                print(split_line)
                old_line = line
            else:
                pairs[urlparse.unquote(split_line[2])] = split_line[4]
                assert(len(split_line) == 6)
    # print data
    # print(pairs)
    return pairs

def parseFiles(dir):
    strings = []
    for files in os.listdir(dir):
        if files.endswith(".ht"):
            string = parseFile(os.path.join(dir,files))
            print(files)
            #print string
            strings.append([files, string])
    return strings

def extractSharedEntries(strings):
    first_dict = strings[0][1]
    shared_dict = {}
    #print(first_dict)
    for key, value in first_dict.iteritems():
        # check that the entry in the same in all dics
        is_in_all_dicts = True
        for dict_file_pair in strings:
            dict = dict_file_pair[1]
            if not dict.has_key(key):
                is_in_all_dicts = False
            elif not dict[key] == value:
                print("Element with different values")
                print(key)
                is_in_all_dicts = False
        if is_in_all_dicts:
            shared_dict[key] = value
    #print(shared_dict)
    for dict_file_pair in strings:
        for key in shared_dict.iterkeys():
            dict_file_pair[1].pop(key)

    strings.append(["shared.ht", shared_dict])
    return strings

def writeOutFiles(dir, strings):
    for string in strings:
        file_name_base = string[0]
        file_name_base = file_name_base.replace(".ht", ".properties")
        file_name = os.path.join(dir, file_name_base)
        file = open(file_name, "w")
        for key, value in string[1].iteritems():
            try:
                file.write(key)
                file.write("=")
                file.write(value)
                file.write("\n")
            except UnicodeDecodeError:
                print(key)
                print(value)
        file.close()

def main (args):
    if(len(args) != 3):
        usage()
        sys.exit(1)

    strings = parseFiles(args[1])
    new_strings = extractSharedEntries(strings)
    writeOutFiles(args[2], new_strings)

if __name__ == "__main__":
    main(sys.argv)
