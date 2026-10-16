#!/usr/bin/env python3
"""Pages and alphanumeric characters of a PDF, in batch-check.sh's own metric."""
import re, subprocess, sys

def measure(path):
    try:
        info = subprocess.run(['pdfinfo', path], capture_output=True, text=True).stdout
        pages = int(info.split('Pages:')[1].split()[0])
    except Exception:
        return None
    text = subprocess.run(['pdftotext', path, '-'], capture_output=True, text=True).stdout
    return pages, len(re.sub(r'[^0-9A-Za-z]', '', text))

if __name__ == '__main__':
    for p in sys.argv[1:]:
        m = measure(p)
        print(f'{p}\t{m[0] if m else "-"}\t{m[1] if m else "-"}')
