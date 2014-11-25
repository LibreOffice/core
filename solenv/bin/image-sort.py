#!/usr/bin/env python

import os
import sys
import re

base_path = ''
global_list = []

def read_icons(fname):
	images = []

	fullpath = base_path + os.pathsep + fname
	if not os.path.exists(fullpath):
		print 'Skipping non-existent ' + fullpath
		return []

	
	fileh = open(fullpath, 'r')
        if not fileh:
		print 'Can\' open ' + fullpath
		os._exit(1)

	for line in fileh:
		match = re.search('xlink:href=\"\.uno:(\S+)\"\s+/', line)
		if match:
			images.append(match.group(1).lower())

	return images

def read_new_icons(fname, prefix):
	images = read_icons(fname)
	new_icons = []

	for icon in images:
		iname = 'cmd/' + prefix + icon + '.png'
		if iname not in new_icons:
			new_icons.append(iname)

	return new_icons

def process_group(prefix, flist):
        global global_list
	for f in flist:
		for icon in read_new_icons(f, prefix):
			if icon not in global_list:
				global_list.append(icon)

	global_list = sorted(global_list, key=str.lower)

def process_file(fname, prefix):
    global global_list
    for i in read_new_icons(fname, prefix):
        global_list.append(i)

def chew_controlfile(filename):
        global global_list

	fname = filename
	flist = []

	with open(filename, 'r') as fileh:
		for line in fileh:
			if re.search('^#', line) or re.search('^\s*$', line):
				continue
			line = re.sub('[\r\n]*$', '', line)

			match = re.search('^-- (\S+)\s*', line)
			if match:
				code = match.group(1).lower()
				is_small = 'small' in line

				if code == 'group':
					if not is_small:
						process_group('lc_', flist)
					process_group('sc_', flist)

				elif code == 'ordered':
					for f in flist:
						if not is_small:
							process_file(f, 'lc_')
						process_file(f, 'sc_')

				elif code == 'literal':
					for f in flist:
						if f not in global_list:
							global_list.append(f)

				else:
					print 'Unkown code "' + code + '"'
					os._exit(1)
						
			else:
				flist.append(line)

if len(sys.argv) < 3:
	print 'image-sort <image-sort.lst> /path/to/libreoffice/source/root'
	os._exit(1)

if len(sys.argv) == 4:
	try:
		sys.stdout = open(sys.argv[3], 'w')
	except Exception, e:
		print 'Can\'t open ' + sys.argv[3] + ': ' + str(e)
		os._exit(1)

lstfile = sys.argv[1]
base_path = sys.argv[2]

chew_controlfile(lstfile)

for x in global_list:
        if not re.search('^sc_.*', x):
            print x

for x in global_list:
        if re.search('^sc_.*', x):
            print x

sys.stdout.close()

# dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
