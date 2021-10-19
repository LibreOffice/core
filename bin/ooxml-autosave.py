#!/usr/bin/python

import os, getopt, sys, shutil
import subprocess

def main(argv):
    inputdir = ''
    outputdir = ''

    #read the arguments
    try:
       opts, args = getopt.getopt(argv,"hi:o:",["idir=","odir="])
    except getopt.GetoptError:
       print ('auto-save.py -i <inputdir> -o <outputdir>')
       sys.exit(2)

    for opt, arg in opts:
       if opt == '-h':
          print ('auto-save.py -i <inputdir> -o <outputdir>')
          sys.exit()
       elif opt in ("-i", "--idir"):
          inputdir = arg
       elif opt in ("-o", "--odir"):
          outputdir = arg


    subdirs = get_list_of_subdir(inputdir)
    for dir in subdirs:
        i = dir.rfind("/")
        extension = dir[i+1:]

        saved_path = os.path.join(outputdir, extension)
        if(os.path.exists(saved_path)):
            shutil.rmtree(saved_path)

        os.makedirs(saved_path)

        file_list = get_list_of_files(dir)

        for file in file_list:

            j = file.rfind("/")
            saved_file_path = os.path.join(saved_path, file[j+1:])

            command = "curl --insecure -F \"file=@"+ file +"\" https://localhost:9980/lool/convert-to/"+extension+" > " + saved_file_path
            os.system(command)

def get_list_of_files(directory_name):

    list_of_file = os.listdir(directory_name)
    all_files = list()

    for filename in list_of_file:
        full_path = os.path.join(directory_name, filename)
        if os.path.isdir(full_path):
            all_files = all_files + get_list_of_files(full_path)
        else:
            all_files.append(full_path)

    return all_files

def get_list_of_subdir(directory_name):

    list_of_file = os.listdir(directory_name)
    subdirs = list()

    for filename in list_of_file:
        full_path = os.path.join(directory_name, filename)
        if os.path.isdir(full_path):
            subdirs.append(full_path)

    return subdirs


if __name__ == "__main__":
    main(sys.argv[1:])