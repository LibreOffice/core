import os
import shutil
from subprocess import call, PIPE, Popen

ROOT_DIR = os.path.abspath(os.pardir)  # to obtain the path
blacklist_path = os.path.join(ROOT_DIR, 'solenv/clang-format/blacklist')
qbfile = open(blacklist_path, "r+")  # to open blacklist txt file
new = qbfile.readlines()
qbfile.seek(0)
for aline in qbfile:  # to loop through the blacklist txt file
    path = aline.rstrip()
    path_file = os.path.join(ROOT_DIR, path)  # to join path
    clang_check = ['clang-format', path_file]
    call(clang_check, stdout=open('demofile.txt', 'w'))  # to call clang-format
    pipe1 = Popen(["diff", "-u", "-s", 'demofile.txt', path_file], stdout=PIPE)
    pipe2 = Popen(["grep", "^+"], stdin=pipe1.stdout, stdout=PIPE)
    pipe3 = Popen(["wc", "-l"], stdin=pipe2.stdout, stdout=PIPE)
    changed_lines = int(pipe3.stdout.read())
    total_lines = sum(1 for line in open(path_file, 'r'))
    percent = int((changed_lines / total_lines) * 100)
    if(changed_lines > 0 and percent <= 5):
        fileA = open('demofile.txt', 'rb')  # open demofile.txt
        fileB = open(path_file, 'wb')
        shutil.copyfileobj(fileA, fileB)
        fileA.close()
        fileB.close()
        for line in new:
            if line.rstrip() != path:
                qbfile.write(line)