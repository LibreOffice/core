token_inc = custom_target('tokeninc',
  input: '../source/token/tokens.txt',
  output: 'token.inc',
  command: [gentokens_bin,
            '@INPUT@',
            '@OUTPUT@',
            meson.current_build_dir() + '/tokennames.inc',
            '@OUTPUT@.perf'])

tokens_hxx = custom_target('token.h',
  output: 'tokens.hxx',
  input: ['../source/token/tokens.hxx.head',
          token_inc,
          '../source/token/tokens.hxx.tail'],
  command: [find_program('cat'),
            '@INPUT@'],
  capture: true)

# FIXME: using a prebuilt .inc file from an existing build.
# The actual generation of which is a horrible shell monstorosity.