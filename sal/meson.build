unix_common = [
  'osl/unx/backtraceapi.cxx',
  'osl/unx/conditn.cxx',
  'osl/unx/file.cxx',
  'osl/unx/file_error_transl.cxx',
  'osl/unx/file_misc.cxx',
  'osl/unx/file_path_helper.cxx',
  'osl/unx/file_stat.cxx',
  'osl/unx/file_url.cxx',
  'osl/unx/file_volume.cxx',
  'osl/unx/interlck.cxx',
  'osl/unx/memory.cxx',
  'osl/unx/module.cxx',
  'osl/unx/mutex.cxx',
  'osl/unx/nlsupport.cxx',
  'osl/unx/pipe.cxx',
  'osl/unx/process.cxx',
  'osl/unx/process_impl.cxx',
  'osl/unx/profile.cxx',
  'osl/unx/random.cxx',
  'osl/unx/readwrite_helper.cxx',
  'osl/unx/salinit.cxx',
  'osl/unx/security.cxx',
  'osl/unx/signal.cxx',
  'osl/unx/socket.cxx',
  'osl/unx/soffice.cxx',
  'osl/unx/tempfile.cxx',
  'osl/unx/thread.cxx',
  'osl/unx/time.cxx',
  'osl/unx/uunxapi.cxx',
]


if host_machine.system() == 'windows'
  system_sources = [
    'osl/w32/backtrace.cxx',
    'osl/w32/interlck.cxx',
    'osl/w32/process.cxx',
    'osl/w32/socket.cxx',
    'osl/w32/conditn.cxx',
    'osl/w32/memory.cxx',
    'osl/w32/procimpl.cxx',
    'osl/w32/tempfile.cxx',
    'osl/w32/dllentry.cxx',
    'osl/w32/module.cxx',
    'osl/w32/profile.cxx',
    'osl/w32/thread.cxx',
    'osl/w32/file.cxx',
    'osl/w32/mutex.cxx',
    'osl/w32/random.cxx',
    'osl/w32/time.cxx',
    'osl/w32/file_dirvol.cxx',
    'osl/w32/nlsupport.cxx',
    'osl/w32/salinit.cxx',
    'osl/w32/file_error.cxx',
    'osl/w32/path_helper.cxx',
    'osl/w32/security.cxx',
    'osl/w32/file_url.cxx',
    'osl/w32/pipe.cxx',
    'osl/w32/signal.cxx',
  ]
  system_link_args = ['ws2_32.lib', 'Mpr.lib', 'userenv.lib', 'Dbghelp.lib', 'Wer.lib']
elif host_machine.system() == 'darwin'
  system_sources = unix_common + ['osl/unx/osxlocale.cxx', 'osl/unx/system.mm']
  system_link_args = []
else
  system_sources = unix_common + ['osl/unx/system.cxx']
  system_link_args = []
endif

sal_lib = shared_library('sal',
  'rtl/alloc_arena.cxx',
  'rtl/alloc_cache.cxx',
  'rtl/alloc_fini.cxx',
  'rtl/alloc_global.cxx',
  'rtl/bootstrap.cxx',
  'rtl/byteseq.cxx',
  'rtl/cipher.cxx',
  'rtl/cmdargs.cxx',
  'rtl/crc.cxx',
  'rtl/digest.cxx',
  'rtl/hash.cxx',
  'rtl/locale.cxx',
  'rtl/math.cxx',
  'rtl/random.cxx',
  'rtl/rtl_process.cxx',
  'rtl/strbuf.cxx',
  'rtl/strimp.cxx',
  'rtl/string.cxx',
#  'rtl/strtmpl.cxx',
  'rtl/unload.cxx',
  'rtl/uri.cxx',
  'rtl/ustrbuf.cxx',
  'rtl/ustring.cxx',
  'rtl/uuid.cxx',
  'osl/all/compat.cxx',
  'osl/all/debugbase.cxx',
  'osl/all/filepath.cxx',
  'osl/all/loadmodulerelative.cxx',
  'osl/all/log.cxx',
  'osl/all/signalshared.cxx',
  'osl/all/utility.cxx',
  system_sources,
  'textenc/context.cxx',
  'textenc/convertbig5hkscs.cxx',
  'textenc/converter.cxx',
  'textenc/converteuctw.cxx',
  'textenc/convertgb18030.cxx',
  'textenc/convertisciidevangari.cxx',
  'textenc/convertiso2022cn.cxx',
  'textenc/convertiso2022jp.cxx',
  'textenc/convertiso2022kr.cxx',
  'textenc/convertsimple.cxx',
  'textenc/convertsinglebytetobmpunicode.cxx',
  'textenc/handleundefinedunicodetotextchar.cxx',
  'textenc/tables.cxx',
  'textenc/tcvtbyte.cxx',
  'textenc/tcvtmb.cxx',
  'textenc/tcvtutf7.cxx',
  'textenc/tcvtutf8.cxx',
  'textenc/tencinfo.cxx',
  'textenc/textcvt.cxx',
  'textenc/textenc.cxx',
  'textenc/unichars.cxx',
  include_directories: [main_inc, 'inc'],
  cpp_args: ['-DSRCDIR="@0@"'.format(meson.current_source_dir()),
             '-DRTL_OS="@0@"'.format(rtl_os),
             '-DRTL_ARCH="x86_64"',
             '-DLIBO_INTERNAL_ONLY',
             '-DSAL_DLLIMPLEMENTATION'],
  link_args: system_link_args,
  dependencies: [thread_dep, osx_frameworks_dep, dl_dep]
  )

if cppunit_dep.found()

  unittester = executable('cppunittester', 'cppunittester/cppunittester.cxx',
    include_directories: [main_inc],
    cpp_args:  ['-DLIBO_INTERNAL_ONLY'],
    link_with: sal_lib,
    dependencies: [cppunit_dep]
    )

  plib = shared_library('osl_test',
    'qa/osl/thread/test_thread.cxx',
    cpp_args:  ['-DLIBO_INTERNAL_ONLY'],
    link_with: sal_lib,
    dependencies: [cppunit_dep, thread_dep],
    include_directories: [main_inc],
  )

  test('sal_test', unittester,
    args: [plib])

  #pc = executable('process_child', 'qa/osl/process/osl_process_child.cxx',
  #  dependencies: [])

  test('osl_security', executable('osl_security', 'qa/osl/security/osl_Security.cxx',
      include_directories: [main_inc],
      cpp_args: ['-DLIBO_INTERNAL_ONLY'],
      link_with: sal_lib,
      dependencies: [cppunit_dep]))
endif
